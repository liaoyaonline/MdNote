# CJSON学习
## 常用函数原型
### 生成json数据
```
cJSON *cJSON_CreateObject(void)
```
* 创建一个`cJSON`文档，返回一个`cJSON`指针
```
#define cJSON_AddStringToObject(object,name,s)  cJSON_AddItemToObject(object, name, cJSON_CreateString(s))
```
* `cJSON_AddStringToObject`函数是整合了`cJSON_AddItemToObject`函数和`cJSON_CreateString`函数，下面看这两个函数的原型
```
void   cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)
```
* 该函数创建一个键值对，其中`object`是`cJSON`指针，`string`是键名，`item`是`JSON`对应的创建值函数，比如:
```
cJSON *cJSON_CreateString(const char *string)
```
* 该函数将一个字符串加入`cJSON`结构体的字符串位置，返回该`cJSON`结构体指针
### 解析json数据
```
cJSON *cJSON_Parse(const char *value) {return cJSON_ParseWithOpts(value,0,0);}
```
* 其中`value`是一个JSON数据包，经过函数`cJSON_Parse`后会返回一个`cJSON`类型指针。
```
char *cJSON_Print(cJSON *item)              {return print_value(item,0,1,0);}
```
* 将`cJSON`数据解析成JSON字符串，并在堆中开辟一块`char*`的内存空间存储`JSON`字符串。
```
cJSON *cJSON_GetObjectItem(cJSON *object,const char *string)
```
* 获取`JSON`字符串字段值，返回一个`cJSON`结构体指针，通过该结构体指针间接访问`JSON`里面的内容

```
void cJSON_Delete(cJSON *c)
```
释放位于堆中`cJSON`结构体内存