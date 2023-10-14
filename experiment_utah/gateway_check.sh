#!/bin/bash
check() {
  STATUS=`lsof -Pi :8080 -sTCP:LISTEN -t`
  if [ "$STATUS" = "" ]; then
    return 1
  fi
  return 0
}

until check; do
  echo "Waiting for gateway to be ready"
  sleep 5
done

echo "gateway ready"

