# alphaFM
## 前言：
* alphaFM是Factorization Machines的一个单机多线程版本实现，用于解决二分类问题，比如CTR预估，优化算法采用了FTRL。
FTRL是一种online learning算法，在Google于2013年给出的论文中用于解决LR的优化，但其实FTRL是一种通用的优化算法，同样可以用于FM。<br>

* 算法原理见我的博客文章：http://castellanzhang.github.io/2016/10/16/fm_ftrl_softmax/

* 在最早写此代码时，正值alphaGo完虐人类，便随手给其取名曰alphaFM。<br>

* 实现alphaFM的初衷是解决大规模数据的FM训练，在我们真实的业务数据中，训练样本数常常是千万到亿级别，特征维度是百万到千万级别甚至上亿，
这样规模的数据完全加载到内存训练已经不太现实，甚至下载到本地硬盘都很困难，一般都是经过spark生成样本直接存储在hdfs上。<br>
alphaFM用于解决这样的问题特别适合，一边从hdfs下载，一边计算，一个典型的使用方法是这样：<br>
训练：10个线程计算，factorization的维度是8，最后得到模型文件fm_model.txt<br>
`hadoop fs -cat train_data_hdfs_path | ./fm_train -core 10 -dim 1,1,8 -m fm_model.txt`<br>
测试：10个线程计算，factorization的维度是8，加载模型文件fm_model.txt，最后输出预测结果文件fm_pre.txt<br>
`hadoop fs -cat test_data_hdfs_path | ./fm_predict -core 10 -dim 8 -m fm_model.txt -out fm_pre.txt`<br>
当然，如果样本文件不大，也可以先下载到本地，然后再运行alphaFM。<br>

* 由于采用了FTRL，调好参数后，训练样本只需过一遍即可收敛，无需多次迭代，因此alphaFM读取训练样本采用了管道的方式，这样的好处除了节省内存，
还可以通过管道对输入数据做各种中间过程的转换，比如采样、格式变换等，无需重新生成训练样本，方便灵活做实验。<br>

* alphaFM还支持加载上次的模型，继续在新数据上训练，理论上可以一直这样增量式进行下去。<br>

* FTRL的好处之一是可以得到稀疏解，在LR上非常有效，但对于FM，模型参数v是个向量，对于每一个特征，必须w为0且v的每一维都为0才算稀疏解，
但这通常很难满足，所以加了一个force_v_sparse的参数，在训练过程中，每当w变成0时，就强制将对应的v变成0向量。这样就可以得到很好的稀疏效果，
且在我的实验中，发现最终对test样本的logloss没有什么影响。<br>

* 当将dim参数设置为1,1,0时，alphaFM就退化成标准的LR的FTRL训练工具。<br>

* 当前版本在v1.0.0基础上做了如下优化，具体见：http://castellanzhang.github.io/2018/09/01/memory_optimization_for_alphafm/
，注意：当前版本只在Linux x86_64系统上通过g++编译测试过，其他环境不保证能够成功编译以及成功执行：
   * 内存优化，相比v1.0.0，fm_train在不改变任何运行参数的情况下内存占用能降到1/3左右，具体降幅取决于特征数据以及-dim参数；fm_predict内存占用降幅更为明显。内存优化带来的益处是显著的，比如对于我们一个典型的应用场景：单机128G内存训练LR模型，可以从原来支持3亿左右的特征维度提升到支持10亿左右。
   * 增加-mnt参数，可以指定内存中模型参数的类型为double还是float，当指定为float时能够进一步降低内存占用，但可能对模型效果有一定影响，谨慎使用。
   * 模型文件可以选择指定为二进制格式，模型加载和输出的速度可以带来10倍量级的提升。
   * 增加模型格式转换工具model_bin_tool，可以输出二进制模型相关信息，可以相互转换二进制模型和文本模型，从二进制转为文本格式时可以选择只保留非0特征。

## 安装方法：
直接在根目录make即可，编译后会在bin目录下生成三个可执行文件。如果编译失败，请升级gcc版本。
## 输入文件格式：
类似于libsvm格式，但更加灵活：特征编号不局限于整数也可以是字符串；特征值可以是整数或浮点数（特征值最好做归一化处理，否则可能会导致结果为nan），
特征值为0的项可以省略不写；正负label可以是1/0或者1/-1。举例如下：<br>
`1 sex:1 age:0.3 f1:1 f3:0.9`<br>
`0 sex:0 age:0.7 f2:0.4 f5:0.8 f8:1`<br>
`...`<br>
## txt模型文件格式：
第一行是bias的参数：<br>
`bias w w_n w_z`<br>
其他行的格式为：<br>
`feature_name w v1 v2 ... vf w_n w_z v_n1 v_n2 ... v_nf v_z1 v_z2 ... v_zf`
## 预测结果格式：
`label score`<br>
其中label为1或-1，score等于预测为正样本的概率值。

## 参数说明：
### fm_train的参数：
-m \<model_path\>: 设置模型文件的输出路径。<br>
-mf \<model_format\>: 设置模型文件的输出格式，txt（文本）或bin（二进制）。	default:txt<br>
-dim \<k0,k1,k2\>: k0为1表示使用偏置w0参数，0表示不使用；k1为1表示使用w参数，为0表示不使用；k2为v的维度，可以是0。	default:1,1,8<br>
-init_stdev \<stdev\>: v的初始化使用均值为0的高斯分布，stdev为标准差。	default:0.1<br>
-w_alpha \<w_alpha\>: w0和w的FTRL超参数alpha。	default:0.05<br>
-w_beta \<w_beta\>: w0和w的FTRL超参数beta。	default:1.0<br>
-w_l1 \<w_L1_reg\>: w0和w的L1正则。	default:0.1<br>
-w_l2 \<w_L2_reg\>: w0和w的L2正则。	default:5.0<br>
-v_alpha \<v_alpha\>: v的FTRL超参数alpha。	default:0.05<br>
-v_beta \<v_beta\>: v的FTRL超参数beta。	default:1.0<br>
-v_l1 \<v_L1_reg\>: v的L1正则。	default:0.1<br>
-v_l2 \<v_L2_reg\>: v的L2正则。	default:5.0<br>
-core \<threads_num\>: 计算线程数。	default:1<br>
-im \<initial_model_path\>: 上次模型的路径，用于初始化模型参数。如果是第一次训练则不用设置此参数。<br>
-imf \<initial_model_format\>: 初始化模型文件的格式，txt（文本）或bin（二进制）。	default:txt<br>
-fvs \<force_v_sparse\>: 为了获得更好的稀疏解。当fvs值为1, 则训练中每当wi = 0，即令vi = 0；当fvs为0时关闭此功能。	default:0<br>
-mnt \<model_number_type\>: 模型参数在内存中和二进制文件中的类型，double或float。	default:double<br>
### fm_predict的参数：
-m \<model_path\>: 模型文件路径。<br>
-mf \<model_format\>: 模型文件格式，txt（文本）或bin（二进制）。	default:txt<br>
-dim \<factor_num\>: v的维度。	default:8<br>
-core \<threads_num\>: 计算线程数。	default:1<br>
-out \<predict_path\>: 输出文件路径。<br>
-mnt \<model_number_type\>: 模型参数在内存中和二进制文件中的类型，double或float。	default:double<br>
### model_bin_tool的参数：
-task \<task_type\>: 1-输出模型信息；2-格式转换，bin到txt；3-格式转换，bin到txt，只保留非零特征；4-格式转换，txt到bin。<br>
-im \<input_model_path\>: 输入模型路径。<br>
-om \<output_model_path\>: 输出模型路径，用于task 2、3和4。task 4必须指定，task 2和3若不指定则默认为标准输出。<br>
-dim \<factor_num\>: v的维度，用于task 4。<br>
-mnt \<model_number_type\>: 模型参数在二进制文件中的类型，用于task 4，double或float。	default:double<br>
## 计算速度：
### 我的实验结果：
本地1000万的样本，200万的特征维度，2.10GHz的CPU，开10个线程，非缺省参数如下：<br>
`-dim 1,1,2 -w_l1 0.05 -v_l1 0.05 -init_stdev 0.001 -w_alpha 0.01 -v_alpha 0.01 -core 10`<br>
训练时间只需要10多分钟。若指定模型文件为二进制格式，速度会更快。

