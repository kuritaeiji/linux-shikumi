#!/bin/bash

while true
do
    date_s=$(date | tr -d '\n')
    info_s=$(ps o pid,comm,vsz,rss,maj_flt,min_flt | grep demand_paging | grep -v grep)

    if [ -z "$info_s" ]
    then
        echo "$date_s: target process seems to be finished"
        break
    fi
    echo "$date_s: $info_s"
    sleep 1
done