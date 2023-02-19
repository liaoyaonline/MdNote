import crudJob from '@/api/system/timing'
调用api/system/timing中的库文件timing.js
import Log from './log'
调用当前文件夹的库文件log.vue
import CRUD, { presenter, header, form, crud } from '@crud/crud'
这个@不知道什么意思，推测调用的文件是src/components/Crud/crud.js的库文件crud.js，但是怎么通过@定位到这个文件的？？？猜测@crud已经被定义为rc/components................
import rrOperation from '@crud/RR.operation'
import crudOperation from '@crud/CRUD.operation'
import pagination from '@crud/Pagination'
以上都是引用rc/components/Crud文件夹里面的几个库文件
import DateRangePicker from '@/components/DateRangePicker'
引用src/components里面的库文件


const defaultForm = { id: null, jobName: null, subTask: null, beanName: null, methodName: null, params: null, cronExpression: null, pauseAfterFailure: true, isPause: false, personInCharge: null, email: null, description: null }
设置一个自定义结构体，结构体内容如上



export default {
  name: 'Timing',
  components: { Log, pagination, crudOperation, rrOperation, DateRangePicker },//这个是注册权限，授予这些库文件里面的函数，定义在本页面运行权限。。。
  cruds() {
    return CRUD({ title: '定时任务', url: 'api/jobs', crudMethod: { ...crudJob }})
  },
  CRUD是库文件CRUD文件中的CRUD函数，其函数定义文件是src/components/Crud/crud.js。。。。。。。
  mixins: [presenter(), header(), form(defaultForm), crud()],
  //这是src/components/Crud/crud.js中的几个函数，其中defaultForm是在本文定义的结构体
  
  
  
  data() {
    return {
      delLoading: false,
      permission: {
        add: ['admin', 'timing:add'],
        edit: ['admin', 'timing:edit'],
        del: ['admin', 'timing:del']
      },
      rules: {
        jobName: [
          { required: true, message: '请输入任务名称', trigger: 'blur' }
        ],
        description: [
          { required: true, message: '请输入任务描述', trigger: 'blur' }
        ],
        beanName: [
          { required: true, message: '请输入Bean名称', trigger: 'blur' }
        ],
        methodName: [
          { required: true, message: '请输入方法名称', trigger: 'blur' }
        ],
        cronExpression: [
          { required: true, message: '请输入Cron表达式', trigger: 'blur' }
        ],
        personInCharge: [
          { required: true, message: '请输入负责人名称', trigger: 'blur' }
        ]
      }
    }
  }
  //填写自定义结构体里面的内容，不过结构体有12个内容，这里只填写了6个，其他的估计是不需要在这里填写，会自行调取数据，比如ID和email等.
  
  
  可以通过/home/liaoya/github/eladmin/eladmin-system/src/main/java/me/zhengjie/modules/quartz/utils/QuartzManage.java
  里面修改里面的几个数据库的存储修改来达到我的目的。
  我的目的只有一个，做一个能够调取