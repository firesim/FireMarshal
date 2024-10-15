#!/bin/bash

perf sched record -- sleep 1
perf sched timehist &> dump.log
cat dump.log
