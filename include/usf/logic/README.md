# 模块说明
    
# logic_executor详细说明
## 执行节点(logic_executor)设计原理

    logic_executor参照 *行为树* 设计的业务逻辑

## 执行节点如何控制执行流程

    行为节点组织为一棵树，正常情况下的流程完全由各个行为节点的返回值和特性控制执行流程。
    也就是说，正常情况下的执行流程是由行为树的配置格式决定的。每一个行为节点不能够、也不应该尝试控制执行流程。

    这里有两个概念需要陈清一下:
    - logic_context_cancel接口
      cancel接口将直接设置整个执行流程的状态，无论当前行为树的执行状态如何，直接跳出整颗树的执行控制。
      一般情况下，这个接口不应该由业务来调用，主要是框架发现特别的情况，比如停止服务等情况下使用。

    - errno
      context上有一个错误码，设置这个错误码不能控制整个执行流程，整个执行流程还是跟没有这个错误码一样的执行下去。
      唯一需要了解的是，当整个执行节点树完成以后，如果返回的结果是执行错误，并且当前没有设置错误码(错误码为0)，
      框架会将错误码设置为-1

## 执行节点返回值说明

  - true 表示节点执行成功
  - false 表示节点执行失败
  - null 表示节点执行异常，具体可以参看数据库对null的定义。
  - redo *action 行为节点* 特有的返回值，用于表示当前执行从挂起回复执行时，当前的节点还需要重入
  
## 执行节点(logic_executor)分类说明

### action 行为节点

    最终实现某一个具体业务的执行节点，由具体的业务实现。

### condition 条件节点

    条件节点包括3个子节点，分别是 if、do、else
    当if条件成立，则执行do分支，否则执行else分支
   
  * 执行 首先执行if节点，根据if节点执行的返回值:
    如果返回*TRUE*，则执行*do*分支，返回*do*分支的执行结果；
    如果返回*FALSE*, 则如果有*else*分支，则执行*else*分支并返回*else*分支结果，如果没有*else*分支，则返回*FALSE*；
    如果返回*NULL*，则不执行任何其他分支，返回*NULL*。
    
  * 返回值 参见执行过程

### decorator 装饰节点

    装饰节点指当前节点下面包含一个子节点。
    如何执行这些节点以及如何返回执行结果是根据不同的装饰节点的类型不同而分别定义的

  - protect 保护节点
    * 执行 简单执行子节点
    * 返回值 无论子节点执行结果(包括*NULL*), 返回*TRUE*

  - is-null 将*NULL*转换为正常的*TRUE*或者*FALSE*
    * 执行 简单执行子节点
    * 返回值 如果子节点执行结果为*NULL*，返回*TRUE*，否则返回*FLASE*
    
  - not 取反节点
    * 执行 简单执行子节点
    * 返回值 将子节点执行结果取反，子节点为*NULL*，保持*NULL*
   
### composite 组合节点

    组合节点指当前节点下面包涵一个执行节点的列表。
    如何调度这些执行节点、如何返回执行结果是根据执行节点的类型不同而分别定义的

  - selector 选择节点
    * 执行 按顺序执行子节点，当遇到第一个子节点返回*TRUE*，则返回*TRUE*。
    * 返回值 有一个子节点返回*TRUE*，则返回*TRUE*，否则(所有子节点执行以后，都没有返回*TRUE*)返回*FALSE*。
      有任何一个子节点返回*NULL*，则直接返回*NULL*
    * 用途 优先决策队列，一般情况下，子节点是一个条件节点，整个结构就可以看成是一个 switch ... case ... 
     
  - sequence 顺序节点
    * 执行 按顺序执行子节点，当遇到第一个子节点返回*FALSE*，则返回*FALSE*。
    * 返回值 有一个子节点返回*FLASE*，则返回*FALSE*，否则(所有子节点执行以后，都没有返回*FALSE*)返回*TRUE*。
      有任何一个子节点返回*NULL*，则直接返回*NULL*
    * 用途 批量执行任务
     
  - parallel 平行节点
    * 执行 执行所有的子节点，而不管兄弟节点的执行情况。节点执行的先后顺序未定义，使用时不应该有任何假设。
      当前实现上采用了按顺序一次执行的策略，后续会修改为并行执行(当某些执行是异步任务时有差别)。
    * 返回值 根据平行节点类型具体不同而不同。
      * *SUCCESS_ON_ALL* 所有子节点成功才算成功
      * *SUCCESS_ON_ONE* 只要有一个子节点成功就算成功
      * 当子节点为空时，认为执行成功
      * 有任何一个子节点返回*NULL*，则返回*NULL*，但是不影响子节点的执行
    * 用途 并行执行多个没有依赖关系的任务

## Action节点的异步执行

## 使用举例
### 通过API直接构造一个执行器

### 通过配置文件构造一个执行器

### 调用一个执行器

## 配置示例
### 配置action行为节点
1. 简单的行节点

```yaml
    action1
```

2. 带参数的行为节点

```yaml
    action2: { arg1: 1, arg2 : 2 }
```

### 配置condition条件节点

1. 完整的条件节点

```yaml
    condition:
        if: action1
        do: action2
        else: action3
```

### 配置decorator装饰节点

1. protect装饰节点配置

```yaml
    protect:
        action1
```

2. not装饰节点配置

```yaml
    not:
        action1
```

### 配置composite组合节点

1. 配置选择节点

```yaml
    selector:
        - condition:
            if: action1
            do: action2
        - condition:
            if: action3
            do: action4
```

1. 配置sequence顺序节点

```yaml
    sequence:
        - action1
        - action2
        - action3
```        

1. 配置sequence顺序节点的简略方法

```yaml
    - action1
    - action2
    - action3
```

1. 配置parallel平行节点的完整方法

```yaml
    parallel:
        policy: SUCCESS_ON_ALL   #或者 SUCCESS_ON_ONE，默认为SUCCESS_ON_ALL
        childs:
            - action1
            - action2
```

1. 配置parallel平行节点的简略方法

```yaml
    parallel:  #policy默认为SUCCESS_ON_ALL
        - action1
        - action2
```
