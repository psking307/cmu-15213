### 微型 Shell（tsh）开发规范

你编写的 tsh Shell 需满足以下特性：

1. 提示符固定为字符串**tsh>**；

2. 用户输入的命令行由一个命令名和零个或多个参数组成，各部分由一个或多个空格分隔。如果命令名是内置命令，tsh 需立即执行并等待下一条命令行；否则，tsh 将该命令名视为可执行文件的路径，在一个初始子进程的上下文中加载并运行该文件（本实验中，作业即指该初始子进程）；

3. tsh 无需支持管道（|）和输入输出重定向（<和>）；

4. 按下 ctrl-c（ctrl-z）时，需向当前前台作业及其所有子进程（如该作业创建的任何子进程）发送 SIGINT（SIGTSTP）信号；若当前无前台作业，该信号不产生任何效果；

5. 若命令行以`&`结尾，tsh 在后台运行该作业；否则在前台运行；

6. 每个作业可通过进程 ID（PID）或作业 ID（JID）标识，作业 ID 由 tsh 分配，为正整数。在命令行中，作业 ID 需以前缀`%`标注，例如`%5`表示作业 ID 为 5，而`5`表示进程 ID 为 5（我们已为你提供了操作作业列表所需的所有函数）；

7. tsh 需支持以下内置命令：

   - quit：终止 Shell；
   - jobs：列出所有后台作业；
   - bg <作业>：向指定作业发送 SIGCONT 信号使其重启，并在后台运行（作业参数可为 PID 或 JID）；
   - fg <作业>：向指定作业发送 SIGCONT 信号使其重启，并在前台运行（作业参数可为 PID 或 JID）；

   

8. tsh 需回收所有僵尸子进程。若某个作业因接收到未捕获的信号而终止，tsh 需识别该事件，并打印包含该作业 PID 和对应信号描述的提示信息。

### 作业验证方法

我们提供了一些工具帮助你验证程序的正确性：

1. **参考解决方案**：Linux 可执行文件 tshref 是本 Shell 的参考实现，运行该程序可解决你关于 Shell 行为的所有疑问。你的 Shell 输出需与参考解决方案完全一致（进程 ID 除外，其每次运行都会变化）；
2. **Shell 驱动程序**：sdriver.pl 程序会将 Shell 作为子进程执行，根据跟踪文件向其发送命令和信号，并捕获并显示 Shell 的输出。

输入`-h`参数可查看 sdriver.pl 的使用方法：

```
unix> ./sdriver.pl -h
用法：sdriver.pl [-hv] -t <跟踪文件> -s <Shell程序> -a <参数>
选项：
-v：输出更详细的信息
-h：打印本帮助信息
-t <跟踪文件>：指定跟踪文件路径
-s <Shell>：指定待测试的Shell程序
-a <参数>：指定传递给Shell的参数
-g：生成供自动评分系统使用的输出
```

我们还提供了 16 个跟踪文件（trace01.txt 至 trace16.txt），你可结合 Shell 驱动程序测试 Shell 的正确性。编号较小的跟踪文件用于简单测试，编号较大的用于复杂测试。

例如，使用 trace01.txt 运行 Shell 驱动程序测试你的 Shell，可输入：

```
unix> ./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
```

（`-a "-p"`参数表示让你的 Shell 不输出提示符），或直接输入：

```
unix> make test01
```

类似地，若要将你的运行结果与参考 Shell 对比，可输入:

```
unix> ./sdriver.pl -t trace01.txt -s ./tshref -a "-p"
```

或

```
unix> make rtest01
```

为方便大家参考，tshref.out 文件包含了参考解决方案在所有跟踪文件上的输出结果，相比手动在所有跟踪文件上运行 Shell 驱动程序，该文件更便捷。

跟踪文件的优势在于，其生成的输出与你交互式运行 Shell 的输出完全一致（仅会多出一行标识跟踪文件的初始注释）。例如：

```
bass> make test15
./sdriver.pl -t trace15.txt -s ./tsh -a "-p"
#
# trace15.txt - 综合测试
#
tsh> ./bogus
./bogus: 命令未找到。
tsh> ./myspin 10
作业(9721)被信号2终止
tsh> ./myspin 3 &
[1] (9723) ./myspin 3 &
tsh> ./myspin 4 &
[2] (9725) ./myspin 4 &
tsh> jobs
[1] (9723) 运行中 ./myspin 3 &
[2] (9725) 运行中 ./myspin 4 &
tsh> fg %1
作业[1] (9723) 被信号20停止
tsh> jobs
[1] (9723) 已停止 ./myspin 3 &
[2] (9725) 运行中 ./myspin 4 &
tsh> bg %3
%3: 无此作业
tsh> bg %1
[1] (9723) ./myspin 3 &
tsh> jobs
[1] (9723) 运行中 ./myspin 3 &
[2] (9725) 运行中 ./myspin 4 &
tsh> fg %1
tsh> quit
bass>
```

### 提示

1. 仔细阅读教材中第 8 章（异常控制流）的每一个内容；

2. 借助跟踪文件推进 Shell 的开发：从 trace01.txt 开始，确保你的 Shell 输出与参考 Shell 完全一致，再依次测试 trace02.txt 及后续文件；

3. `waitpid`、`kill`、`fork`、`execve`、`setpgid`和`sigprocmask`函数会起到很大作用，`waitpid`的`WUNTRACED`和`WNOHANG`选项也非常实用；

4. 实现信号处理函数时，务必向整个前台进程组发送 SIGINT 和 SIGTSTP 信号，在`kill`函数的参数中使用`-pid`而非`pid`，sdriver.pl 程序会测试该细节是否正确；

5. 本次作业的难点之一是划分waitfg和sigchld_handler函数的功能，我们推荐以下实现方式：

   - 在`waitfg`中，基于`sleep`函数实现忙等待循环；
   - 在sigchld_handler中，仅调用一次waitpid。当然也有其他实现方案（如在两个函数中都调用waitpid），但这类方案极易造成逻辑混乱，所有子进程的回收工作都在信号处理函数中完成会更简洁；

6. 在eval函数中，父进程在创建子进程前，需调用sigprocmask阻塞 SIGCHLD 信号，在调用addjob将子进程加入作业列表后，再调用sigprocmask解除阻塞。由于子进程会继承父进程的信号阻塞集，子进程在调用exec加载新程序前，必须解除对 SIGCHLD 信号的阻塞。父进程通过这种方式阻塞 SIGCHLD 信号，是为了避免出现竞态条件：子进程被sigchld_handler回收（并从作业列表中移除）后，父进程才调用addjob将其加入作业列表；

7. `more`、`less`、`vi`和`emacs`等程序会对终端设置进行特殊修改，不要在你编写的 Shell 中运行这些程序，选择`/bin/ls`、`/bin/ps`和`/bin/echo`等简单的文本类程序即可；

8. 当你在标准 Unix Shell 中运行自己编写的 Shell 时，你的 Shell 会处于前台进程组。若你的 Shell 创建子进程，默认情况下该子进程也会属于前台进程组。由于按下 ctrl-c 会向前台进程组的所有进程发送 SIGINT 信号，这会导致你的 Shell 及其创建的所有进程都接收到该信号，显然这是不符合预期的。

   解决方法：在fork创建子进程后、execve加载程序前，子进程调用setpgid(0, 0)，该函数会将子进程放入一个新的进程组，该进程组的组 ID 与子进程的 PID 相同。这能确保前台进程组中只有你的 Shell 一个进程，当按下 ctrl-c 时，Shell 捕获产生的 SIGINT 信号，再将其转发给对应的前台作业（更准确地说，是包含前台作业的进程组）。

### 评分标准

本次作业满分 90 分，评分分布如下：

- 正确性：80 分，16 个跟踪文件，每个 5 分；
- 代码风格：10 分，其中规范的注释 5 分、检查所有系统调用的返回值 5 分。

我们会在 Linux 机器上，使用实验目录中提供的 Shell 驱动程序和跟踪文件测试你的 Shell 程序，你的 Shell 在这些跟踪文件上的输出需与参考 Shell 完全一致，仅以下两种情况除外：

1. 进程 ID 可以（且必然）不同；
2. trace11.txt、trace12.txt 和 trace13.txt 中`/bin/ps`命令的输出会随运行次数变化，但输出中所有 mysplit 进程的运行状态必须与参考结果一致。

### 笔记

#### 需要完成的函数:

~~~c
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
~~~

#### 提供给我的功能函数：

~~~ c
// 解析用户输入的命令行字符串 cmdline：
// 1. 拆分命令参数到 argv 数组（支持单引号包裹的参数，比如 'hello world' 会被识别为单个参数）；
// 2. 识别命令是否以 & 结尾，返回 1 表示后台作业（BG），返回 0 表示前台作业（FG）；
// 3. 自动忽略空行、首尾空格，处理参数分隔符。
int parseline(const char *cmdline, char **argv); 

// SIGQUIT 信号处理器：收到 SIGQUIT 时，打印 “Terminating after receipt of SIGQUIT signal” 并退出 shell（用于驱动程序优雅终止 shell）。
void sigquit_handler(int sig);

// 清空单个作业结构体 job 的所有字段：PID 置 0、JID 置 0、状态置 UNDEF、命令行清空，用于初始化或删除作业。
void clearjob(struct job_t *job);

// 初始化整个作业列表 jobs：遍历 MAXJOBS 个作业项，逐个调用 clearjob，让列表回到空状态。
void initjobs(struct job_t *jobs);

//遍历作业列表，返回当前已分配的最大作业 ID（JID），用于分配新 JID（nextjid）时参考。
int maxjid(struct job_t *jobs); 

// 向作业列表添加新作业：
// 1. 找列表中第一个空位置（PID=0）；
// 2. 填充 PID、状态（FG/BG/ST）、命令行、分配新 JID（nextjid 自增）；
// 3. 返回 1 表示添加成功，0 表示失败（比如列表满）；
// 4. 若开启 verbose，打印添加日志。
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);

// 根据 PID 从作业列表删除指定作业：
// 1. 找到 PID 匹配的作业项，调用 clearjob 清空；
// 2. 更新 nextjid 为当前最大 JID+1；
// 3. 返回 1 表示删除成功，0 表示未找到该 PID。
int deletejob(struct job_t *jobs, pid_t pid); 

// 遍历作业列表，返回当前前台作业的 PID；若没有前台作业，返回 0（核心：shell 处理 ctrl-c/ctrl-z 时，需要知道该给谁发信号）。
pid_t fgpid(struct job_t *jobs);

// 根据 PID 查找作业列表中的作业结构体，返回指向该结构体的指针；未找到返回 NULL（比如 fg/bg 命令要根据 PID 找作业）。
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);

// 根据 JID（作业 ID，如 %1）查找作业结构体，返回指针；未找到返回 NULL（处理 fg %1 这类按 JID 操作的命令）。
struct job_t *getjobjid(struct job_t *jobs, int jid); 

// 将进程 PID 映射为对应的作业 JID：遍历列表找到 PID 匹配的作业，返回其 JID；未找到返回 0（打印作业列表时，需要 PID 转 JID）。
int pid2jid(pid_t pid); 

// 打印整个作业列表：遍历所有非空作业项，输出格式为 [JID] (PID) 状态 命令行（比如 [1] (1234) Running ls -l），对应 jobs 内置命令。
void listjobs(struct job_t *jobs);

// 打印 shell 的使用帮助信息（-h/-v/-p 参数说明），然后退出程序（用户输入无效启动参数时调用）。
void usage(void);

// Unix 系统调用错误处理：打印自定义错误信息 msg + 系统错误描述（strerror(errno)），然后退出程序（比如 fork/waitpid 失败时调用）。
void unix_error(char *msg);

// 应用层错误处理：仅打印自定义错误信息 msg，然后退出程序（非系统调用错误，比如 fgets 读取出错）。
void app_error(char *msg);

// 信号处理器函数的类型定义：声明 handler_t 是 “接收一个 int 参数、返回 void” 的函数类型（简化信号处理函数的声明）。
typedef void handler_t(int);

// 信号注册的封装函数（封装 sigaction）：
// 1. 为信号 signum 设置处理器 handler；
// 2. 阻塞当前处理的信号（避免嵌套触发）；
// 3. 设置 SA_RESTART 标志（系统调用被信号中断后自动重启）；
// 4. 返回旧的信号处理器；
// 5. 若 sigaction 失败，调用 unix_error 退出（替代原生 signal 函数，更健壮）。
handler_t *Signal(int signum, handler_t *handler);
~~~

#### void eval(char *cmdline)实现

eval是这个简易 shell（tsh）的**核心执行引擎**，它接收用户输入的命令行字符串（比如 `ls -l`、`sleep 10 &`、`fg %1`），完成以下核心工作：

1. 解析命令行，区分内置命令（`quit`/`jobs`/`bg`/`fg`）和外部命令（`ls`/`sleep` 等）；
2. 内置命令：直接在 shell 进程内执行（无需创建子进程）；
3. 外部命令：创建子进程，在子进程中执行命令，并管理子进程的运行状态（前台 / 后台）；
4. 关键细节：为子进程设置独立的进程组，避免后台进程被 `ctrl-c`/`ctrl-z` 误杀。

简单说：`eval` 是 “用户输入” 到 “命令执行” 的核心桥梁，决定了命令**怎么执行、在前台还是后台执行、由谁执行（shell 自身 / 子进程）**。

它的首要任务是调用 parseline 函数, 这个函数解析了以空格分隔的命令行参数，并构造最终会传递给 execve 的 argv 向量。 第一个参数被假设为要么是一个内置的 shell 命令名，马上就会解释这个命令，要么是一个可执行目标文件，会在一个新的子进程的上下文中加载并运行这个文件。

如果最后一个参数是一个"&"字符，那么 parseline返回 1, 表示应该在后台执行 该程序(shell 不会等待它完成）。否则，它返回 o, 表示应该在前台执行这个程序(shell会等待它完成）。 在解析了命令行之后， eval 函数调用 builtin_comrnand 函数，该函数检查第一个命令行参数是否是一个内置的 shell 命令。如果是，它就立即解释这个命令，并返回值1。否则返回 0。 简单的 shell 只有一个内置命令——-quit 命令，该命令会终止 shell。实际使用的 shell 有大量的命令，比如 pwd、 jobs 和 fg。 如果 builtin—command 返回 o, 那么 shell 创建一个子进程，并在子进程中执行所请求的程序。如果用户要求在后台运行该程序，那么 shell 返回到循环的顶部，等待下一个命令行。否则， shell 使用 waitpid函数等待作业终止。当作业终止时， shell 就开始下一轮迭代。