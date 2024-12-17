#!/bin/bash

# note: grpc work for the async
ghz --insecure --async --proto message/reader.proto --call reader.ReaderService.GetRecordString -c 1 -n 100 --rps 1 -d '{"key": "location1", "value": "\"Jacobs\""}' 0.0.0.0:12423
