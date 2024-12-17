#!/bin/bash

wrk -t1 -c1 -d100 -s benchmark/seq.lua http://localhost:8080/