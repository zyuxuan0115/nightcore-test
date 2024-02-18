/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef UniqueIdService_H
#define UniqueIdService_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "social_network_types.h"

namespace social_network {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class UniqueIdServiceIf {
 public:
  virtual ~UniqueIdServiceIf() {}
  virtual void UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier) = 0;
};

class UniqueIdServiceIfFactory {
 public:
  typedef UniqueIdServiceIf Handler;

  virtual ~UniqueIdServiceIfFactory() {}

  virtual UniqueIdServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(UniqueIdServiceIf* /* handler */) = 0;
};

class UniqueIdServiceIfSingletonFactory : virtual public UniqueIdServiceIfFactory {
 public:
  UniqueIdServiceIfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf>& iface) : iface_(iface) {}
  virtual ~UniqueIdServiceIfSingletonFactory() {}

  virtual UniqueIdServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(UniqueIdServiceIf* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> iface_;
};

class UniqueIdServiceNull : virtual public UniqueIdServiceIf {
 public:
  virtual ~UniqueIdServiceNull() {}
  void UploadUniqueId(const int64_t /* req_id */, const PostType::type /* post_type */, const std::map<std::string, std::string> & /* carrier */) {
    return;
  }
};

typedef struct _UniqueIdService_UploadUniqueId_args__isset {
  _UniqueIdService_UploadUniqueId_args__isset() : req_id(false), post_type(false), carrier(false) {}
  bool req_id :1;
  bool post_type :1;
  bool carrier :1;
} _UniqueIdService_UploadUniqueId_args__isset;

class UniqueIdService_UploadUniqueId_args {
 public:

  UniqueIdService_UploadUniqueId_args(const UniqueIdService_UploadUniqueId_args&);
  UniqueIdService_UploadUniqueId_args& operator=(const UniqueIdService_UploadUniqueId_args&);
  UniqueIdService_UploadUniqueId_args() : req_id(0), post_type((PostType::type)0) {
  }

  virtual ~UniqueIdService_UploadUniqueId_args() throw();
  int64_t req_id;
  PostType::type post_type;
  std::map<std::string, std::string>  carrier;

  _UniqueIdService_UploadUniqueId_args__isset __isset;

  void __set_req_id(const int64_t val);

  void __set_post_type(const PostType::type val);

  void __set_carrier(const std::map<std::string, std::string> & val);

  bool operator == (const UniqueIdService_UploadUniqueId_args & rhs) const
  {
    if (!(req_id == rhs.req_id))
      return false;
    if (!(post_type == rhs.post_type))
      return false;
    if (!(carrier == rhs.carrier))
      return false;
    return true;
  }
  bool operator != (const UniqueIdService_UploadUniqueId_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UniqueIdService_UploadUniqueId_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class UniqueIdService_UploadUniqueId_pargs {
 public:


  virtual ~UniqueIdService_UploadUniqueId_pargs() throw();
  const int64_t* req_id;
  const PostType::type* post_type;
  const std::map<std::string, std::string> * carrier;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _UniqueIdService_UploadUniqueId_result__isset {
  _UniqueIdService_UploadUniqueId_result__isset() : se(false) {}
  bool se :1;
} _UniqueIdService_UploadUniqueId_result__isset;

class UniqueIdService_UploadUniqueId_result {
 public:

  UniqueIdService_UploadUniqueId_result(const UniqueIdService_UploadUniqueId_result&);
  UniqueIdService_UploadUniqueId_result& operator=(const UniqueIdService_UploadUniqueId_result&);
  UniqueIdService_UploadUniqueId_result() {
  }

  virtual ~UniqueIdService_UploadUniqueId_result() throw();
  ServiceException se;

  _UniqueIdService_UploadUniqueId_result__isset __isset;

  void __set_se(const ServiceException& val);

  bool operator == (const UniqueIdService_UploadUniqueId_result & rhs) const
  {
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const UniqueIdService_UploadUniqueId_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UniqueIdService_UploadUniqueId_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _UniqueIdService_UploadUniqueId_presult__isset {
  _UniqueIdService_UploadUniqueId_presult__isset() : se(false) {}
  bool se :1;
} _UniqueIdService_UploadUniqueId_presult__isset;

class UniqueIdService_UploadUniqueId_presult {
 public:


  virtual ~UniqueIdService_UploadUniqueId_presult() throw();
  ServiceException se;

  _UniqueIdService_UploadUniqueId_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class UniqueIdServiceClient : virtual public UniqueIdServiceIf {
 public:
  UniqueIdServiceClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  UniqueIdServiceClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier);
  void send_UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier);
  void recv_UploadUniqueId();
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class UniqueIdServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (UniqueIdServiceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_UploadUniqueId(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  UniqueIdServiceProcessor(::apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> iface) :
    iface_(iface) {
    processMap_["UploadUniqueId"] = &UniqueIdServiceProcessor::process_UploadUniqueId;
  }

  virtual ~UniqueIdServiceProcessor() {}
};

class UniqueIdServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  UniqueIdServiceProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< UniqueIdServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< UniqueIdServiceIfFactory > handlerFactory_;
};

class UniqueIdServiceMultiface : virtual public UniqueIdServiceIf {
 public:
  UniqueIdServiceMultiface(std::vector<apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~UniqueIdServiceMultiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> > ifaces_;
  UniqueIdServiceMultiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<UniqueIdServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->UploadUniqueId(req_id, post_type, carrier);
    }
    ifaces_[i]->UploadUniqueId(req_id, post_type, carrier);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class UniqueIdServiceConcurrentClient : virtual public UniqueIdServiceIf {
 public:
  UniqueIdServiceConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  UniqueIdServiceConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier);
  int32_t send_UploadUniqueId(const int64_t req_id, const PostType::type post_type, const std::map<std::string, std::string> & carrier);
  void recv_UploadUniqueId(const int32_t seqid);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _MSC_VER
  #pragma warning( pop )
#endif

} // namespace

#endif
