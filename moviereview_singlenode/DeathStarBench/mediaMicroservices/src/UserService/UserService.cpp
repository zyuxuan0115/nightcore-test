#include "../utils.h"
#include "../ThriftSeverEventHandler.h"
#include "../ClientPool.h"
#include "../MCClient.h"
#include "../ThriftClient.h"
#include "../utils_mongodb.h"
#include "UserHandler.h"

using namespace media_service;

static json config_json;
static ClientPool<MCClient>* mc_client_pool;
static mongoc_client_pool_t *mongodb_client_pool;
static std::string machine_id;
static std::mutex* thread_lock;

int faas_init() {
  init_logger();

  SetUpTracer("config/jaeger-config.yml", "user-service");

  if (load_config(&config_json) != 0) {
    return -1;
  }

  std::string mc_addr = config_json["user-memcached"]["addr"];
  int mc_port = config_json["user-memcached"]["port"];
  mc_client_pool = new ClientPool<MCClient>("user-memcached", mc_addr, mc_port, 1, 128, 1000);
  mongodb_client_pool = init_mongodb_client_pool(config_json, "user", 128);

  if (mongodb_client_pool == nullptr) {
    return -1;
  }

  if (GetMachineId(&machine_id) != 0) {
    return -1;
  }

  thread_lock = new std::mutex;

  return 0;
}

int faas_create_func_worker(void* caller_context,
                            faas_invoke_func_fn_t invoke_func_fn,
                            faas_append_output_fn_t append_output_fn,
                            void** worker_handle) {
    FaasWorker* faas_worker = new FaasWorker(
        caller_context, invoke_func_fn, append_output_fn);
  
    std::string secret = config_json["secret"];

  int port = 9090;
  std::string compose_addr = config_json["compose-review-service"]["addr"];
  int compose_port = config_json["compose-review-service"]["port"];
  std::string compose_http_path = config_json["compose-review-service"]["http_path"];

  auto compose_client_pool = new ClientPool<ThriftClient<ComposeReviewServiceClient>>(
      "compose-review-client", compose_addr, compose_port, 0, 128, 1000, "ComposeReviewService",
      faas_worker, "UserService", "ComposeReviewService");

  faas_worker->SetProcessor(
      std::make_shared<UserServiceProcessor>(
          std::make_shared<UserHandler>(
              thread_lock,
              machine_id,
              secret,
              mc_client_pool,
              mongodb_client_pool,
              compose_client_pool)));
    *worker_handle = faas_worker;
    return 0;
}

int faas_destroy_func_worker(void* worker_handle) {
    FaasWorker* faas_worker = reinterpret_cast<FaasWorker*>(worker_handle);
    delete faas_worker;
    return 0;
}

int faas_func_call(void* worker_handle,
                   const char* input, size_t input_length) {
    FaasWorker* faas_worker = reinterpret_cast<FaasWorker*>(worker_handle);
    bool success = faas_worker->Process(input, input_length);
    return success ? 0 : -1;
}