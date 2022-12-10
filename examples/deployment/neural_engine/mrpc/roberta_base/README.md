# Step-by-Step

# Prerequisite

### 1. Installation
1.1 Install python environment
Create a new python environment
```shell
conda create -n <env name> python=3.8
conda activate <env name>
```
Check the gcc version using $gcc-v, make sure the gcc version is higher than 9.0.
If not, you need to update gcc by yourself.
Make sure you have the autoconf installed.
If not, you need to install autoconf by yourself.
Make sure the cmake version is 3 rather than 2.
If not, you need to install cmake.
```shell
cmake --version
conda install cmake
sudo apt install autoconf
```
Install Intel Extension for Transformers from Source Code
```shell
cd <intel_extension_for_transformers_folder>
git submodule update --init --recursive
python setup.py install
```
Install package for examples
```shell
cd <intel_extension_for_transformers_folder>/examples/deployment/neural_engine/mrpc/roberta_base
pip install -r requirements.txt
```
1.2 Environment variables Preload libjemalloc.so can improve the performance when multi instance.
```
export LD_PRELOAD=<intel_extension_for_transformers_folder>/intel_extension_for_transformers/backends/neural_engine/executor/third_party/jemalloc/lib/libjemalloc.so
```
Using weight sharing can save memory and improve the performance when multi instance.
```
export WEIGHT_SHARING=1
export INST_NUM=<inst num>
```
### 2. Prepare Dataset and pretrained model

### 2.1 Get dataset

```shell
python prepare_dataset.py --dataset_name=glue --task_name=mrpc --output_dir=./data
```

### 2.2 Get model
Neural Engine can parse Tensorflow/Pytorch/ONNX model and Neural Engine IR.  
You can get FP32 ONNX model from optimization module by setting precision=fp32, command as follows:
```shell
bash prepare_model.sh --input_model=Intel/roberta-base-mrpc  --task_name=mrpc --output_dir=./model_and_tokenizer --precision=fp32
```
Throught setting precision=int8 you could get PTQ int8 model and setting precision=bf16 to get bf16 model.
```shell
bash prepare_model.sh --input_model=Intel/roberta-base-mrpc  --task_name=mrpc --output_dir=./model_and_tokenizer --precision=int8
```

### Benchmark

  2.1 accuracy  
  run python
  ```shell
  GLOG_minloglevel=2 python run_executor.py --input_model=./model_and_tokenizer/int8-model.onnx  --tokenizer_dir=./model_and_tokenizer --mode=accuracy --data_dir=./data --batch_size=8
  ```
  or run shell
  ```shell
  bash run_benchmark.sh --input_model=./model_and_tokenizer/int8-model.onnx  --tokenizer_dir=./model_and_tokenizer --mode=accuracy --data_dir=./data --batch_size=8
  ```

  2.2 performance  
  run python
  ```shell
  GLOG_minloglevel=2 python run_executor.py --input_model=./model_and_tokenizer/int8-model.onnx --mode=performance --batch_size=8 --seq_len=128
  ```
  or run shell
  ```shell
  bash run_benchmark.sh --input_model=./model_and_tokenizer/int8-model.onnx  --mode=performance --batch_size=8 --seq_len=128
  ```
  or compile framwork model to IR using python API
  ```
  from intel_extension_for_transformers.backends.neural_engine.compile import compile
  graph = compile('./model_and_tokenizer/int8-model.onnx')
  graph.save('./ir')
  ```
  and run C++  
  The warmup below is recommended to be 1/10 of iterations and no less than 3.
  ```
  export GLOG_minloglevel=2
  export OMP_NUM_THREADS=<cpu_cores>
  export DNNL_MAX_CPU_ISA=AVX512_CORE_AMX
  export UNIFIED_BUFFER=1
  numactl -C 0-<cpu_cores-1> <intel_extension_for_transformers_folder>/intel_extension_for_transformers/backends/neural_engine/bin/neural_engine
  --batch_size=<batch_size> --iterations=<iterations> --w=<warmup>
  --seq_len=128 --config=./ir/conf.yaml --weight=./ir/model.bin
  ```
