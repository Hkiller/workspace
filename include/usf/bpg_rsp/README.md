# 模块说明

## 模块配置说明

 - type: bpg_rsp_manage

 - logic-manage: GameLogicManager
 
 -  executor-manage: GameLogicExecutorMgr

 - commit-to
 
     请求执行完成后，构造的响应送到什么函数作下一个处理。

 - rsps-load-from: /rsps

 - rsps-load-meta: <string>

    如果需要使用宏的名字配置各个响应器，则需要配置一个元数据库，宏名字到命令吗的转换从元数据中读取。
    元数据库从dr_store模块获取，此处配置的是名字
    
 - rsps-load-meta-store-manage

    加载元数据库的管理器，一般不需要配置，系统中应该只存在一个元数据库管理器。如果需要，则在此配置管理器模块名字
 
 - logic-queue
 
     所有Rsp执行时排队的逻辑队列配置。如果不配置队列，所有的Rsp在请求到达后就开始执行，不会排队保证先后顺序。
     可以配置多个队列，用于排序逻辑上不同的请求。具体例子见下
     
    - { name: op, scope: client, max-count: 10, is-default: 1 }
    - { name: tick, scope: client, max-count: 1 }

       name： 队列名字，在Rsp配置使用什么队列时关联使用
       scope: client 每个client来的请求排队
              global 全局排队
       max-count: 队列中最大请求个数，当超过时直接忽略
       is-default: 每一个rsp-manage只能有一个默认队列的配置，当RSP上没有配置队列信息时，采用这个队列。

