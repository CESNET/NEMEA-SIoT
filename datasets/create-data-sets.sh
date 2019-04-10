#!/bin/bash


src_joy_dir="joy-data/ipcam"
dst_data_set_dir="data-sets/ipcam"
joy="../joy/sleuth"

echo $src_joy_dir
echo $dst_data_set_dir

for dir in $src_joy_dir/*
do
 dir_name=$(basename -- "$dir")
 for f in $dir/*
 do
  echo "Processing $f .." 
  filename=$(basename -- "$f")
  extension="${filename##*.}"
  name="${filename%.*}"
 #call for ikea-app
# $joy $f --where "da=192.168.3.213 | sa=192.168.3.213" > "$dst_data_set_dir/$dir_name/$name.json"

 #call for ikea-homekit (anomalies)
 #$joy $f --where "da=192.168.3.213 | sa=192.168.3.213" > "$dst_data_set_dir/$dir_name/$name.json"

# call for http anomaly
# $joy $f --where "da=192.168.5.69 | sa=192.168.5.69" > "$dst_data_set_dir/$dir_name/$name.json"

 #call for ipcam, http anomaly and google-mini
 $joy $f > "$dst_data_set_dir/$dir_name/$name.json"
 done
done


