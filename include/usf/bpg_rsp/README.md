# ģ��˵��

## ģ������˵��

 - type: bpg_rsp_manage

 - logic-manage: GameLogicManager
 
 -  executor-manage: GameLogicExecutorMgr

 - commit-to
 
     ����ִ����ɺ󣬹������Ӧ�͵�ʲô��������һ������

 - rsps-load-from: /rsps

 - rsps-load-meta: <string>

    �����Ҫʹ�ú���������ø�����Ӧ��������Ҫ����һ��Ԫ���ݿ⣬�����ֵ��������ת����Ԫ�����ж�ȡ��
    Ԫ���ݿ��dr_storeģ���ȡ���˴����õ�������
    
 - rsps-load-meta-store-manage

    ����Ԫ���ݿ�Ĺ�������һ�㲻��Ҫ���ã�ϵͳ��Ӧ��ֻ����һ��Ԫ���ݿ�������������Ҫ�����ڴ����ù�����ģ������
 
 - logic-queue
 
     ����Rspִ��ʱ�Ŷӵ��߼��������á���������ö��У����е�Rsp�����󵽴��Ϳ�ʼִ�У������Ŷӱ�֤�Ⱥ�˳��
     �������ö�����У����������߼��ϲ�ͬ�����󡣾������Ӽ���
     
    - { name: op, scope: client, max-count: 10, is-default: 1 }
    - { name: tick, scope: client, max-count: 1 }

       name�� �������֣���Rsp����ʹ��ʲô����ʱ����ʹ��
       scope: client ÿ��client���������Ŷ�
              global ȫ���Ŷ�
       max-count: ������������������������ʱֱ�Ӻ���
       is-default: ÿһ��rsp-manageֻ����һ��Ĭ�϶��е����ã���RSP��û�����ö�����Ϣʱ������������С�

