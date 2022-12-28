#!/bin/bash
# set -x

export GLOG_minloglevel=2
batch_size=1
seq_len=384
warm_up=5
iteration=10
max_eval_samples=-1

function main {
  init_params "$@"
  run_benchmark
}

# init params
function init_params {
  for var in "$@"
  do
    case $var in
      --input_model=*)
          input_model=$(echo $var |cut -f2 -d=)
      ;;
      --mode=*)
          mode=$(echo $var |cut -f2 -d=)
      ;;
      --batch_size=*)
          batch_size=$(echo $var |cut -f2 -d=)
      ;;
      --seq_len=*)
          seq_len=$(echo $var |cut -f2 -d=)
      ;;
      --warm_up=*)
          warm_up=$(echo $var |cut -f2 -d=)
      ;;
      --iteration=*)
          iteration=$(echo $var |cut -f2 -d=)
      ;;
      --data_dir=*)
          data_dir=$(echo $var |cut -f2 -d=)
      ;;
      --max_eval_samples=*)
          max_eval_samples=$(echo $var |cut -f2 -d=)
      ;;
    esac
  done
}

# run accuracy
function run_benchmark {
    python run_executor.py \
      --input_model=${input_model} \
      --mode=$mode \
      --batch_size=${batch_size} \
      --seq_len=${seq_len} \
      --warm_up=${warm_up} \
      --iteration=${iteration} \
      --data_dir=${data_dir} \
      --max_eval_samples=${max_eval_samples}\
      --dataset_name=squad \
      --tokenizer_dir=./model_and_tokenizer
}

main "$@"