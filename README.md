lv1 lv2的大框架在实验文档https://pku-minic.github.io/online-doc/#/lv2-code-gen/code-gen中都有，一般只需要补充/...的内容即可
lv3并没有大框架qwq 只是告诉要修改ast 文档开头给出的规范告诉了要添加的种类
lv4 IR生成不需要任何修改，目标代码生成也无需变更，修改ast，
变量新增alloc，load和store，栈分配也要改
lv5 比较难，实现了很久
lv6 if else的实现参考了网上借助优先级来处理二义性的方法，在risv实现上第11个点不过，树洞说是imm12，但是解决不出来，后续lv7也没报错就放弃了
lv7 在完成lv5和lv6后挺简单的？只需要维护while的栈结构就好