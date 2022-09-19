## Qt
### Qt基础知识

#### 自动内存回收

Qt拥有自己的内存回收机制，可以自动回收内存，实例化的对象不需要手动delete()。

Qt一般会形成对象树，所创建的对象一般在堆区，当析构父类对象时子类对象也会析构，回收时一并回收。

#### 坐标系

- 左上角是坐标(0,0)；
- x以右为正方向；
- y以下为正方向。

### 创建项目

点击Create Project...

![image-20220919172336677](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\创建1)

选择Widgets。

![image-20220919172534596](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\创建2)

继续。

![image-20220919172624950](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\创建3)

接下来有三个选项，默认创建的窗口类是`QMainWindow`，可以选择的基类有： `QWidget`、`QMainWindow`、`QDialog`，一个是单一的窗口，一个是主界面，一个是对话框，`MainWindow`比`Widget`多了一些常用的控件，例如菜单栏。

![image-20220919172748900](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\创建4)

接下来一直下一步即可，最终生成：

![image-20220919173027901](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\创建5)

1. `main.cpp`：如下

```c
    QApplication a(argc, argv);    	//应用程序对象，有且仅有一个
    MainWindow w;    				//实例化窗口对象
    w.show();   			 		//调用show函数 显示窗口

    return a.exec();    			//让应用程序对象进入消息循环机制中，代码阻塞到当前行
```

2. `mainwindow.cpp`：这个就是用来书写逻辑代码的主要地方，和`mainwindow.h`是一起的

3. `mainwindow.ui`：ui文件，可视化界面，用于做界面布局。

### 帮助文档

值得一提的是，Qt自带了help文档，可以查阅自己需要的功能（虽然是英文）。

![image-20220919174916037](D:\3D Foundation\3DGraphicsFoundamentals\Homework1\帮助1)

顺便一提`QtCreator`本身也是可以用来文本编辑的，只不过不太好用。

### 信号和槽
信号和槽是Qt的一个特色，当一个类运行的时候，调用信号，就会触发相应的槽函数，同时信号函数还可以传递参数给槽函数。

#### 用法
连接函数 ：connect(参数1, 参数2, 参数3, 参数4)

- 参数1 信号的发送者
- 参数2 发送的信号（函数地址）
- 参数3 信号的接受者
- 参数4 处理的槽函数 （函数的地址）

大家可以点击所给的Project文件夹里的`viewwidget.h`看看哪些是信号哪些是槽。

#### 自定义信号与槽函数
有的时候，只用类自带的信号与槽比较局限，我们还可以用自己定义的信号和槽函数。

##### 自定义信号

自定义的信号要写到头文件下的 signals下，有时候项目没自动生成，自己写上，返回值为void，只需要声明，不需要实现。
可以有参数，同时也可以重载已有的信号。
##### 自定义槽函数
自定义的槽函数，返回值为void，需要声明 ，也需要实现，可以有参数 ，可以重载，要写在头文件下的 `public slot:`下 或者`public:` 或者全局函数。
