UnrealGo
===========

UnrealGo is a experimental implementation of [alphagoZero](https://deepmind.com/blog/alphago-zero-learning-scratch/) in C++. It's developed with MCTS
and deep reinforcement learning. It can be trained from scratch.

Dependencies
=============
Boost - 1.64+ <br/>
http://www.boost.org/ <br/>

```
# build boost from source
$ ./bootstrap.sh
$ ./b2
# set LIB_INSTALL_DIR
$ cp -rf stage/lib/ ${LIB_INSTALL_DIR}
# edit and add lib directory to file /etc/ld.so.conf
$ sudo ldconfig
```

ZMQ - 4.2.2
```
$ git clone https://github.com/zeromq/libzmq.git
$ sudo apt-get install autogen autoconf libtool
$ ./autogen.sh
$ ./configure
$ ./make
$ ./make install
```

build tensorflow cmake interface
==============
[tensorflow cmake interface](https://github.com/unrealgo/tensorflow_cmake_interface)<br>
Assume $PROJ_WORKSPACE is your project home directory
```
# clone tensorflow
$ cd $PROJ_WORKSPACE
$ git clone https://github.com/tensorflow/tensorflow.git
# git checkout to desired version(e.g. 1.6) if you like

# build interface
$ cd $PROJ_WORKSPACE
$ git clone https://github.com/unrealgo/tensorflow_cmake_interface.git
$ cd tensorflow_cmake_interface
$ mkdir build
$ cd build
$ cmake -DTENSORFLOW_SHARED=ON -DTENSORFLOW_SOURCE_DIR="$PROJ_WORKSPACE/tensorflow" ..
$ make
$ make install
```
After finishing build, the interface is installed at TensorflowCC="$PROJ_WORKSPACE/tensorflow_cmake_interface/build/lib/cmake/TensorflowCC"

Build UnrealGo
===========
Build UnrealGo with tensorflow cmake interface:
```
$ git clone https://github.com/unrealgo/unrealgo.git
$ mkdir build
$ cd build
$ cmake -DTensorflowCC="$PROJ_WORKSPACE/tensorflow_cmake_interface/build/lib/cmake/TensorflowCC" ../
$ make -j4
```

Run UnrealGo
===========
The binary is installed at $PROJ_WORKSPACE/unrealgo/bin.
To run unrealgo, the network checkpoint(weights) is needed. A bootstrap checkpoint of 20 residual block is provided,
decompress and copy to bin/

```
# first copy config/dlcfg-19 to bin/
$ cd $PROJ_WORKSPACE/
$ cp config/dlcfg-19 bin/

$ tar xvf bootstrap.tar.gz
$ mv bootstrap-ckpt.* bin/

# start unrealgo, type selfplay to start self play.
$ ./unrealgo
 selfplay
```
Config
=============
meta_graph: path to metagraph <br>
checkpoint: prefix of checkpoint files <br>
nn_input: input name of the model inputs <br>
nn_outputs: policy:value name pairs of the model outputs <br>
value_transform: transform formula for network value output <br>
```
0: v = v
1: v = -v
2: v = 1-v
```
reuse_searchtree: whether to reuse search tree

GUI
=============
[Sabaki](https://github.com/SabakiHQ/Sabaki/releases)
```
$ git clone https://github.com/SabakiHQ/Sabaki.git
$ cd Sabaki/dist/linux-unpacked
$ ./sabaki
# after sabaki is started, select [Engines]->[Attach] to attach unrealgo binary.
```

Neural Network Training
===========
The hard part is neural network training.  UnrealGo run in client mode by default, allowing user to run command 'selfplay' or 'clienteval'
1. command 'selfplay' executes mcts search by single player with identical network weight
2. command 'clienteval' executes mcts search by two players with different network weights.<br>

The generated selfplay data or evaluation result will be synced to server for training/update.

License
===========
UnrealGo is licensed under GPLv3 or later.
