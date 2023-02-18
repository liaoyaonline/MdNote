# C++让Vector的数组随机排列
## 思路

**输入:**&emsp; vector\<string> Array
**过程:**&emsp;vector\<string> Shuffle_ArrayOrder(vector\<string> Array_tmp)
1:&emsp;新建一个map<int,int>map_tmp ,其中first从0到Array.size(),second是10000  <br/> &emsp;&emsp;以内的随机数
2:&emsp;对map_tmp根据second进行排序
3:&emsp;新建一个vector\<string>Array_Result,遍历map_tmp,把Array[map_tmp[i]]<br/> &emsp;&emsp;依次压入。
**输出:** &emsp;Array_Result

## 代码
```cpp

vector<string> Shuffle_ArrayOrder(vector<string> Array_tmp)
{
    srand((int)time(0));
    for(int i = 0; i < Array_tmp.size(); i++)
        cout << Array_tmp[i] << "---";
    map<int, int> NumSignMap_tmp;
    for(int i = 0; i < Array_tmp.size(); i++)
    {
        int Num_random = (rand() % 10000);
        NumSignMap_tmp[i] = Num_random;
    }
    for(int i = 0; i < Array_tmp.size(); i++)
    {
        cout << i << ": " << NumSignMap_tmp[i] << endl;
    }
    vector<pair<int, int>> VectorMap_tmp;
    for (auto it = NumSignMap_tmp.begin(); it != NumSignMap_tmp.end(); it++)
        VectorMap_tmp.push_back(make_pair(it->first, it->second));

    sort(VectorMap_tmp.begin(), VectorMap_tmp.end(),
        [](const pair<int, int> &x, const pair<int, int> &y) -> int {
        return x.second < y.second;
    });

    for (auto it = VectorMap_tmp.begin(); it != VectorMap_tmp.end(); it++)
        cout << it->first << ':' << it->second << '\n';
    vector<string> Array_result = Array_tmp;
    auto it = VectorMap_tmp.begin();
    for(int i = 0; i < Array_result.size(); i++)
    {
        Array_result[i] = Array_tmp[it->first];
        it++;
    }
    cout << endl << endl;
    for(int i = 0; i < 11; i++)
    {
        cout << Array_result[i] << "----";
    }
    cout << endl;
    return Array_result;
}
```