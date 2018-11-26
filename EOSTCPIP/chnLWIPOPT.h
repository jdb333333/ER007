/**************************************************************************************************************
在这里先说一下这两个配置lwip协议栈文件opt.h和lwipopts.h的关系：
  opt.h是lwip“出厂”时原装的配置文件，它的作者是瑞士科学院的Adam等人，而lwipopts.h的作者是stellarisWare
的工程师，它集合了opt.h中常常需要改动的部分和针对Stellaris所特有的配置选项添加进来了。这两个文件里边
都配置的选项，以后者为准，不是共有的选项以它们各自的配置为准。

在这里先说一下lwip的内存管理机制，我们以enet_lwip这个例程为例。
    在使用lwip的时候，我们可以使用两种形式的内存，一种是heap，一种是pool。heap就像是一整块蛋糕，我们需要
多少就切多少，但 是切了之后不能吃，只能看，因为看完之后，你还要放回去让别人看，因为当整块蛋糕很少的时候，
有很多人等着都要切了看，这样很多次的切了之后又放回去，必 然要产生一些很小的蛋糕块，这就是内存的碎片，
到最后都是切得很细的蛋糕块，假如这个时候你想切块大点的蛋糕来看看，不好意思，你找遍了整个拼凑起来的蛋
糕块，没有发现你想要的那么大的，结果你只好放弃了，所以最后内存申请回频频失败。

    因为我们是嵌入式系统， 我们的RAM再大也不能和PC机的天文数字相比，我们不能使用PC机所使用的蛋糕刀具来切蛋糕，
必须使用更小型的道具，占用CODE更小，这一点Adam 等相对于标准C而实现了几个小的内存分配、重分配和释放函数，
它们都以mem_为前缀，已和原来的标准库函数相区别。当然如果你偏要使用大的刀，只需要 #define MEM_LIBC_MALLOC 1 。

   这几个工具不管你是heap型模式切蛋糕还是pool模式切蛋糕，都可以用它们来完成，它们只是刀，至于怎么切蛋糕，
这里不是它们该考虑的事情。

   上面介绍了heap型的切蛋糕的方法，在简单说一下pool型切蛋糕的方法。
   为了能让更多的人把蛋糕切回去看并且不至于还回来的蛋糕都是非常地小。Adam非常努力地在lwip切蛋糕上问题上
采用一种Pool型的切法，找来一块蛋 糕，把它切成n等分，每一份都是相同的固定大小，一份不够的可以拿两份，
一份用起来太多的人你也最少拿一份（虽然浪费了，但是你也必须这么做），这种切法 有两个好处，一是：现在你不
能再随便的按自己的要求大小去切蛋糕了，大小是固定的，这样你申请了蛋糕之后不需要去切了，已经切好了，你可以
很快地拿到一份 大小固定的蛋糕，这非常适合于在接受数据帧的底层使用，当大量的数据来的时候，我们一下子需要
很多的内存，这种方式下，我们很快就可以拿到足量的内存，不 至于应接不暇；二是：在切成多少份，每份多大上需
要我们根据最常接受到的数据帧的大小来费点脑筋，切得太小了，内存的利用率就下降了，切得太大了，我们就 会严
重地浪费内存，如果总共可切的内存还不大的话，那肯定是要因为后面来的数据无蛋糕可切而要丢包的，这就像是现在
的房地产，少数富人拥有好几套大房子空 着不住人，而另一方面，多数穷人却因为租不起房子而只好离开大城市，但
是当前穷人对于城市所作的贡献并不见得比富人小。

    这两种切法都不是完美的，也不可能有完美的，但是有它们最适合的地方。我上面的比喻是为了更形象地理解，
当然在细节上可能会有不同，不过，大体上就是这么个意思，细节我们可以去深入地分析。

    Pool型一般用于从链路层就收一个帧，目的是快速、少碎片、浪费一点没关系、只要我能满足大多数人迅速地有房可住即可。

    heap型一般用于缓存应用层生成的数据，大小自己定，相切多少切多少，随心所欲，尽管以后可能没得切，但我只在乎现在，
也许以后就有新的方法解决呢，就像煤炭和石油，该用还得用，说不定以后会有新能源取代它们，杞人忧天干嘛。

    实际上，在enet_lwip这个例程里它就是这么实现的。

    如何配置heap的大小呢，
#define MEM_SIZE                        (12 * 1024)

        所以说当你有大量的数据要发送的时候，你最好把这个值设置得大一些。否则你会因为切不到heap中的内存而无法发送或只能延迟发送。
        最好是把上面这个数设置成4字节对齐的。

   如何配置pool的大小呢，
#define PBUF_POOL_SIZE                  16
#define PBUF_POOL_BUFSIZE               256

        第一个是我们要切的分数，第二个是每一份的大小。
        那么实际上我们切到的每一份内存的大小要比我们定义的大16个字节。我们可以根据应用去调整这几个值，
        这两部分的大小就占到了lwip协议栈所占内存的大部分，也就是说他们会影响到ZI段的大小。

        如果要使发送的应用程序也采用pool的方式而不是heap，则要：
        #define MEM_USE_POOLS 1
        #define MEM_USE_CUSTOM_POOLS 1
        并且还要在工程所在目录下创建文件 lwippools.h，里包括：
        LWIP_MALLOC_MEMPOOL_START
        LWIP_MALLOC_MEMPOOL(20, 256)
        LWIP_MALLOC_MEMPOOL(10, 512)
        LWIP_MALLOC_MEMPOOL(5, 1512)
        LWIP_MALLOC_MEMPOOL_END
        这样协议栈所涉及到的内存都用POOL方式来管理了，这种方法在StellarisWare例程中一般没有采用。
        如果使用POOL,以下关于内存的两个函 数是不会被调用的： mem_init()；这个函数主要是对堆内存的初始化，并返回指针。
        mem_realloc(mem, size)；这个函数对已分配的对内存块进行收缩。

****************************************************************************************************************/
//*****************************************************************************
//
// lwipopts.h - Configuration file for lwIP
//*************************************************************************
//
// NOTE:  This file has been derived from the lwIP/src/include/lwip/opt.h
// header file.
// 注:此文件起源于opt.h
// For additional details, refer to the original "opt.h" file, and lwIP
// documentation.
// 详情参考opt.h。
//*****************************************************************************

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

//*****************************************************************************
//
// ---------- Stellaris / lwIP Port Options ----------
//
//*****************************************************************************
#define HOST_TMR_INTERVAL               100         // 主机定时器间隔，注意不是lwip协议栈本身的定时器间隔，可用来检查IP地址的获取情况或者周期性地调用一些函数。
//#define DHCP_EXPIRE_TIMER_MSECS         (10 * 1000) //DHCP 获取超时的毫秒数。这个实际在例程中没看到有用到。DHCP获取IP超时，一般就会采用AUTOIP的方式自给IP。
//#define INCLUDE_HTTPD_SSI                           //在HTTPD中如果有含有SSI标签的网页存在，则开启它，否则如果是普通的html网页，不需要开启。
//#define INCLUDE_HTTPD_CGI                            //如果需要CGI的处理，则需要开启。
//#define DYNAMIC_HTTP_HEADERS                         //如果要动态的添加HTTP首部则开启，在使用makefsfile 带参数-h时，就去掉了http首部，如果不能正常浏览到网页可尝试开启。一般不需要开启，样可以节省Flash的空间，因为如果不是使用SD卡，而是使用内部文件系统的时候，所有的网页数据都放在了RO段。

//*****************************************************************************
//
// ---------- Platform specific locking ----------
//
//*****************************************************************************
#define SYS_LIGHTWEIGHT_PROT            1           // default is 0      针对Stellaris必须1，主要是因为在分配内存的时候，要确保总中断关闭。防止内存分配失败。
#define NO_SYS                          1           // default is 0   如果为使用RTOS，就置1.
//#define MEMCPY(dst,src,len)             memcpy(dst,src,len)   /* 该宏用来定义我们是否需要C语言标准库函数memcpy(),如果有更有效的函数，该宏可以忽略.不适用C标准库
//#define SMEMCPY(dst,src,len)            memcpy(dst,src,len)   //同上

//*****************************************************************************
//
// ---------- Memory options ----------
//
//*****************************************************************************
//#define MEM_LIBC_MALLOC                 0          //如果为1，就表示我们使用c库的 malloc/free/realloc，否则使用lwip自带的函数，注意加了前缀的。
#define MEM_ALIGNMENT                    4           //Stellaris该值必须为4，设置CPU的对齐方式
#define MEM_SIZE                        (12 * 1024)  // default is 1600     该值在ZI中占了很大的份额。这就是堆内存的大小，如果应用程序有大量数据在发送是要被复制，那么该值就应该尽量大一点。由此可见，发送缓冲区从这里边分配。

//#define MEMP_OVERFLOW_CHECK             0         //是否开启内存POOL溢出检查，即是否使能堆内存溢出检查.
//#define MEMP_SANITY_CHECK               0         //设置为1表示在每次调用函数memp_free()后，进行一次正常的检查，以确保链表队列没有循环
//#define MEM_USE_POOLS                   0         //是否使用POOL型内存来作为发送缓冲，而不是heap型，如果开启的话，可能还要创建头文件lwippool.h
//#define MEMP_USE_CUSTOM_POOLS           0         //内存Pool是否使用固定大小的POOL，开启这个前提是要开启上面的。

//*****************************************************************************
//
// ---------- Internal Memory Pool Sizes ----------
//
//*****************************************************************************
#define MEMP_NUM_PBUF                     20  //来自memp的PBUF_ROM和PBUF_REF类型的数目，如果应用程有大量的数据来自ROM或者静态mem的数据要发送，此值要设大一些。
//#define MEMP_NUM_RAW_PCB                4   //原始连接（就是应用程不经过传输层直接到IP层获取数据）PCB的数目，该项依赖lwip_raw项的开启。
//#define MEMP_NUM_UDP_PCB                4   //UDP的PCB数目，每一活动的UDP “连接” 需要一个PCB。
#define MEMP_NUM_TCP_PCB                  3   // 同时建立激活的TCP连接的数目(要求参数LWIP_TCP使能).默认为5 我改成1之后和原来的8没有什么区别。但是这里建立tcp连接发送数据之后就立刻关闭了tcp连接.
                                            //如果这里设置为1，就要注意了，我们在点亮led的时候实际上是几乎同时发送了两个GET请求，要求建立两个激活的tcp连接，如果设置为1，就会等到一个tcp激活的tcp连接关闭之后
                                            //再建立一个新的tcp连接，所以才会出现延迟返回状态的现象。经过我的实验，发现等于3的时候比较特殊，会使code少4个字节。
                                            //而且ZI的大小也只比2的时候多160,（奇数多160，偶数时多168）。
#define MEMP_NUM_TCP_PCB_LISTEN           1     /* 能够监听的TCP连接数目(要求参数LWIP_TCP使能).默认为8我改成了1之后对本例程也是无影响的。这个非常规律，多一个ZI就多32个字节。*/
#define MEMP_NUM_TCP_SEG                  20  //最多同时在队列的TCP_SEG的数目.这个数为20，MEMP_NUM_TCP_PCB为3时，code会比其它情况要小4个字节，其它所有值时code大小不变，当从偶数增加到奇数时，ZI增加24，从奇数增加到偶数时ZI增加16。
//#define MEMP_NUM_REASSDATA              5  // 最多同时在队列等待重装的IP包数目，是整个IP包，不是IP分片。
//#define MEMP_NUM_ARP_QUEUE              30 //
//#define MEMP_NUM_IGMP_GROUP             8  //
//#define MEMP_NUM_SYS_TIMEOUT            3  // 能够同时激活的timeout的个数(要求NO_SYS==0)。默认为3
//#define MEMP_NUM_NETBUF                 2  // netbufs结构的数目，仅当使用sequential API的时候需要。默认为2
//#define MEMP_NUM_NETCONN                4  // netconns结构的数目，仅当使用sequential API的时候需要。默认为4
//#define MEMP_NUM_TCPIP_MSG_API          8  // tcpip_msg结构的数目，它用于callback/timeout API的通信(仅当使用tcpip.c的时候需要 )。默认为8
//#define MEMP_NUM_TCPIP_MSG_INPKT        8  // 接收包时tcpip_msg结构体的数目。


//*****************************************************************************
//
// ---------- ARP options ----------
//
//*****************************************************************************
//#define LWIP_ARP                        1   //开启ARP
//#define ARP_TABLE_SIZE                  10  //ARP表项的大小。激活的MAC-IP地址对存储区的数目
//#define ARP_QUEUEING                    1   //设置为1表示在硬件地址解析期间，将发送数据包放入到队列中
//#define ETHARP_TRUST_IP_MAC             1

//*****************************************************************************
//
// ---------- IP options ----------
//
//*****************************************************************************
//#define IP_FORWARD                      0
//#define IP_OPTIONS_ALLOWED              1
#define IP_REASSEMBLY                   0           // default is 1 注意进来的IP分段包就不会被重装，所以大于1500的IP包可能会有些意想不到的问题
#define IP_FRAG                         0           // default is 1    这样从这里发送出去的包不会被分片。这个不会出现问题，因为我们的TCP_MSS才512
//#define IP_REASS_MAXAGE                 3
//#define IP_REASS_MAX_PBUFS              10
//#define IP_FRAG_USES_STATIC_BUF         1
//#define IP_FRAG_MAX_MTU                 1500
//#define IP_DEFAULT_TTL                  255

//*****************************************************************************
//
// ---------- ICMP options ----------
//
//*****************************************************************************
//#define LWIP_ICMP                       1
//#define ICMP_TTL                       (IP_DEFAULT_TTL)

//*****************************************************************************
//
// ---------- RAW options ----------
//
//*****************************************************************************
//#define LWIP_RAW                        1
//#define RAW_TTL                        (IP_DEFAULT_TTL)

//*****************************************************************************
//
// ---------- DHCP options ----------
//
//*****************************************************************************
#define LWIP_DHCP                       1           // default is 0
//#define DHCP_DOES_ARP_CHECK             ((LWIP_DHCP) && (LWIP_ARP))

//*****************************************************************************
//
// ---------- UPNP options ----------
//
//*****************************************************************************
//#define LWIP_UPNP                       0

//*****************************************************************************
//
// ---------- PTPD options ----------
//
//*****************************************************************************
//#define LWIP_PTPD                       0

//*****************************************************************************
//
// ---------- AUTOIP options ----------
//
//*****************************************************************************
#define LWIP_AUTOIP                     1           // default is 0
#define LWIP_DHCP_AUTOIP_COOP           ((LWIP_DHCP) && (LWIP_AUTOIP))
                                                    // default is 0
#define LWIP_DHCP_AUTOIP_COOP_TRIES     5           // default is 9

//*****************************************************************************
//
// ---------- SNMP options ----------
//
//*****************************************************************************
//#define LWIP_SNMP                       0
//#define SNMP_CONCURRENT_REQUESTS        1
//#define SNMP_TRAP_DESTINATIONS          1
//#define SNMP_PRIVATE_MIB                0
//#define SNMP_SAFE_REQUESTS              1

//*****************************************************************************
//
// ---------- IGMP options ----------
//
//*****************************************************************************
//#define LWIP_IGMP                       0

//*****************************************************************************
//
// ---------- DNS options -----------
//
//*****************************************************************************
//#define LWIP_DNS                        0
//#define DNS_TABLE_SIZE                  4
//#define DNS_MAX_NAME_LENGTH             256
//#define DNS_MAX_SERVERS                 2
//#define DNS_DOES_NAME_CHECK             1
//#define DNS_USES_STATIC_BUF             1
//#define DNS_MSG_SIZE                    512

//*****************************************************************************
//
// ---------- UDP options ----------
//
//*****************************************************************************
//#define LWIP_UDP                        1
//#define LWIP_UDPLITE                    0
//#define UDP_TTL                         (IP_DEFAULT_TTL)

//*****************************************************************************
//
// ---------- TCP options ----------
//
//*****************************************************************************
//#define LWIP_TCP                        1
//#define TCP_TTL                         (IP_DEFAULT_TTL)
#define TCP_WND                         2048    // default is 2048, was 4096  改变该值并不影响code和ZI的大小。
//#define TCP_MAXRTX                      12
//#define TCP_SYNMAXRTX                   6
//#define TCP_QUEUE_OOSEQ                 1
#define TCP_MSS                         512     // default is 128, was 1500       改变该值并不影响code和ZI的大小。。该值规定了TCP数据包数据部分的最大值，不包括tcp首部
//#define TCP_CALCULATE_EFF_SEND_MSS      1
#define TCP_SND_BUF                     (6 * TCP_MSS)    //改变该值并不影响ZI的大小,但稍稍影响code大小，几个字节。
                                                    // default is 256
//#define TCP_SND_QUEUELEN                (4 * (TCP_SND_BUF/TCP_MSS))
//#define TCP_SNDLOWAT                    (TCP_SND_BUF/2)
//#define TCP_LISTEN_BACKLOG              0
//#define TCP_DEFAULT_LISTEN_BACKLOG      0xff

//*****************************************************************************
//
// ---------- API options ----------
//
//*****************************************************************************
//#define LWIP_EVENT_API                  0
//#define LWIP_CALLBACK_API               1

//*****************************************************************************
//
// ---------- Pbuf options ----------
//
//*****************************************************************************
#define PBUF_LINK_HLEN                  16          // default is 14  改成16是因为在Stellaris系列中，FIFO中的帧是开始有两个字节的帧长度，针对Stellaris必须16
#define PBUF_POOL_SIZE                  16            // 奇数时code比偶数时多4个字节，每+1，RAM多消耗272个字节。这也就是说每个pbuf需要272个字节，而每一个pbuf
                                                    //由两部分组成，一部分是缓冲区256个字节，一部分是pbuf首部（16个字节。）（不是以太网链路层的帧首部，尽管它从FIFO中是16个字节）。这个pbuf就是直接装入从RX FIFO中传
                                                    //过来的数据。每一个pbuf可以存一个帧，可以存256个字节的一个帧。这部分内存主要用来接收的。
#define PBUF_POOL_BUFSIZE               256            //这个pbuf包括前边的16个字节的pbuf头，叫首部有点不合适，这个pbuf头里保存这个pbuf的所有信息。
                                                    // default is LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_HLEN)
#define ETH_PAD_SIZE                    2           // default is 0      针对Stellaris必须为2

//*****************************************************************************
//
// ---------- Network Interfaces options ----------
//
//*****************************************************************************
//#define LWIP_NETIF_HOSTNAME             0
//#define LWIP_NETIF_API                  0
//#define LWIP_NETIF_STATUS_CALLBACK      0
//#define LWIP_NETIF_LINK_CALLBACK        0
//#define LWIP_NETIF_HWADDRHINT           0

//*****************************************************************************
//
// ---------- LOOPIF options ----------
//
//*****************************************************************************
//#define LWIP_HAVE_LOOPIF                0
//#define LWIP_LOOPIF_MULTITHREADING      1

//*****************************************************************************
//
// ---------- Thread options ----------
//
//*****************************************************************************
//#define TCPIP_THREAD_NAME              "tcpip_thread"
//#define TCPIP_THREAD_STACKSIZE          0
//#define TCPIP_THREAD_PRIO               1
//#define TCPIP_MBOX_SIZE                 0
//#define SLIPIF_THREAD_NAME             "slipif_loop"
//#define SLIPIF_THREAD_STACKSIZE         0
//#define SLIPIF_THREAD_PRIO              1
//#define PPP_THREAD_NAME                "pppMain"
//#define PPP_THREAD_STACKSIZE            0
//#define PPP_THREAD_PRIO                 1
//#define DEFAULT_THREAD_NAME            "lwIP"
//#define DEFAULT_THREAD_STACKSIZE        0
//#define DEFAULT_THREAD_PRIO             1
//#define DEFAULT_RAW_RECVMBOX_SIZE       0
//#define DEFAULT_UDP_RECVMBOX_SIZE       0
//#define DEFAULT_TCP_RECVMBOX_SIZE       0
//#define DEFAULT_ACCEPTMBOX_SIZE         0

//*****************************************************************************
//
// ---------- Sequential layer options ----------
//
//*****************************************************************************
//#define LWIP_TCPIP_CORE_LOCKING         0
#define LWIP_NETCONN                    0           // default is 1

//*****************************************************************************
//
// ---------- Socket Options ----------
//
//*****************************************************************************
#define LWIP_SOCKET                     0           // default is 1
//#define LWIP_COMPAT_SOCKETS             1
//#define LWIP_POSIX_SOCKETS_IO_NAMES     1
//#define LWIP_TCP_KEEPALIVE              0
//#define LWIP_SO_RCVTIMEO                0
//#define LWIP_SO_RCVBUF                  0
//#define SO_REUSE                        0

//*****************************************************************************
//
// ---------- Statistics options ----------
//
//*****************************************************************************
//#define LWIP_STATS                      1
//#define LWIP_STATS_DISPLAY              0
//#define LINK_STATS                      1
//#define ETHARP_STATS                    (LWIP_ARP)
//#define IP_STATS                        1
//#define IPFRAG_STATS                    (IP_REASSEMBLY || IP_FRAG)
//#define ICMP_STATS                      1
//#define IGMP_STATS                      (LWIP_IGMP)
//#define UDP_STATS                       (LWIP_UDP)
//#define TCP_STATS                       (LWIP_TCP)
//#define MEM_STATS                       1
//#define MEMP_STATS                      1
//#define SYS_STATS                       1

//*****************************************************************************
//
// ---------- PPP options ----------
//
//*****************************************************************************
//#define PPP_SUPPORT                     0
//#define PPPOE_SUPPORT                   0
//#define PPPOS_SUPPORT                   PPP_SUPPORT

#if PPP_SUPPORT
//#define NUM_PPP                         1
//#define PAP_SUPPORT                     0
//#define CHAP_SUPPORT                    0
//#define MSCHAP_SUPPORT                  0
//#define CBCP_SUPPORT                    0
//#define CCP_SUPPORT                     0
//#define VJ_SUPPORT                      0
//#define MD5_SUPPORT                     0
//#define FSM_DEFTIMEOUT                  6
//#define FSM_DEFMAXTERMREQS              2
//#define FSM_DEFMAXCONFREQS              10
//#define FSM_DEFMAXNAKLOOPS              5
//#define UPAP_DEFTIMEOUT                 6
//#define UPAP_DEFREQTIME                 30
//#define CHAP_DEFTIMEOUT                 6
//#define CHAP_DEFTRANSMITS               10
//#define LCP_ECHOINTERVAL                0
//#define LCP_MAXECHOFAILS                3
//#define PPP_MAXIDLEFLAG                 100

//#define PPP_MAXMTU                      1500
//#define PPP_DEFMRU                      296
#endif

//*****************************************************************************
//
// ---------- checksum options ----------
//
//*****************************************************************************
//#define CHECKSUM_GEN_IP                 1
//#define CHECKSUM_GEN_UDP                1
//#define CHECKSUM_GEN_TCP                1
//#define CHECKSUM_CHECK_IP               1
//#define CHECKSUM_CHECK_UDP              1
//#define CHECKSUM_CHECK_TCP              1

//*****************************************************************************
//
// ---------- Debugging options ----------
//
//*****************************************************************************
#if 0
#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
extern void UARTprintf(const char *pcString, ...);
#define LWIP_PLATFORM_DIAG(x) {UARTprintf x;}
#define LWIP_DEBUG
#endif


//#define   LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_WARNING
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_SERIOUS
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_SEVERE

//#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
#define LWIP_DBG_TYPES_ON               (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH)

//#define ETHARP_DEBUG                    LWIP_DBG_ON     // default is OFF
//#define NETIF_DEBUG                     LWIP_DBG_ON     // default is OFF
//#define PBUF_DEBUG                        LWIP_DBG_ON
//#define API_LIB_DEBUG                   LWIP_DBG_OFF
//#define API_MSG_DEBUG                   LWIP_DBG_OFF
//#define SOCKETS_DEBUG                   LWIP_DBG_OFF
//#define ICMP_DEBUG                      LWIP_DBG_OFF
//#define IGMP_DEBUG                      LWIP_DBG_OFF
//#define INET_DEBUG                      LWIP_DBG_OFF
//#define IP_DEBUG                        LWIP_DBG_ON     // default is OFF
//#define IP_REASS_DEBUG                  LWIP_DBG_OFF
//#define RAW_DEBUG                       LWIP_DBG_OFF
//#define MEM_DEBUG                       LWIP_DBG_OFF
//#define MEMP_DEBUG                      LWIP_DBG_OFF
//#define SYS_DEBUG                       LWIP_DBG_OFF
//#define TCP_DEBUG                       LWIP_DBG_OFF
//#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
//#define TCP_FR_DEBUG                    LWIP_DBG_OFF
//#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
//#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
//#define TCP_WND_DEBUG                   LWIP_DBG_OFF
//#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
//#define TCP_RST_DEBUG                   LWIP_DBG_OFF
//#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
//#define UDP_DEBUG                       LWIP_DBG_ON     // default is OFF
//#define TCPIP_DEBUG                     LWIP_DBG_OFF
//#define PPP_DEBUG                       LWIP_DBG_OFF
//#define SLIP_DEBUG                      LWIP_DBG_OFF
//#define DHCP_DEBUG                      LWIP_DBG_ON     // default is OFF
//#define AUTOIP_DEBUG                    LWIP_DBG_OFF
//#define SNMP_MSG_DEBUG                  LWIP_DBG_OFF
//#define SNMP_MIB_DEBUG                  LWIP_DBG_OFF
//#define DNS_DEBUG                       LWIP_DBG_OFF
#define HTTPD_DEBUG                        LWIP_DBG_ON
#endif /* __LWIPOPTS_H__ */


//ccr2017-11-09*******************************************************************************************

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

//*****************************************************************************
//
// ---------- SNMP options ----------
//
//*****************************************************************************
#define LWIP_SNMP                       0      　　//SNMP协议启用
#if LWIP_SNMP
#define SNMP_CONCURRENT_REQUESTS     1            //SNMP当前允许请求数量
#define SNMP_TRAP_DESTINATIONS       1            //SNMP trap目的地址数目
#define SNMP_PRIVATE_MIB             1      　　　 //SNMP 私有节点设置允许
#define SNMP_SAFE_REQUESTS           1            //仅回复SNMP安全请求
#endif

//*****************************************************************************
//
// ---------- Platform specific locking ----------
//
//*****************************************************************************
#define SYS_LIGHTWEIGHT_PROT              1          //临界中断保护开关(多任务模式下开启)
#define NO_SYS                            0          //LWIP独立运行或者基于操作系统，为0则基于操作系统

//*****************************************************************************
//
// ---------- Memory options ----------
//
//*****************************************************************************
#define MEM_LIBC_MALLOC                 0                //采用LWIP自带函数分配动态内存
#define MEM_LIBC_MALLOC                 0                //内存池不通过内存堆来分配
#define MEM_ALIGNMENT                   4                //字节对齐(和CPU的位数有关,32位设置为4)
#define MEM_SIZE                        8*1024  　　　　　//堆内存大小，用于发送数据
#define MEMP_SANITY_CHECK               0                //mem_free调用后检查链表是否有循环 by zc
#define MEMP_OVERFLOW_CHECK             0                //lwip堆内存溢出检查
#define MEM_USE_POOLS                   0                //内存堆不通过内存池分配
#define MEM_USE_POOLS_TRY_BIGGER_POOL   0             　 //申请内存失败不选择大内存池
#define MEMP_USE_CUSTOM_POOLS           0                //同上

//*****************************************************************************
//
// ---------- Internal Memory Pool Sizes ----------
//
//*****************************************************************************
#define MEMP_NUM_PBUF                       60    　　//来自memp的PBUF_ROM(ROM内存池)和PBUF_REF(RAM内存池)数目最大总和
#define MEMP_NUM_RAW_PCB                    4    　　 //RAW连接的PCB数目(需要LWIP RAW有效)
#define MEMP_NUM_UDP_PCB                    4        //能够同时连接的UDP的PCB数目
#define MEMP_NUM_TCP_PCB                    (TCP_WND + TCP_SND_BUF)/TCP_MSS  //能够同时连接的TCP的PCB数目 12
#define MEMP_NUM_TCP_PCB_LISTEN              1       //(TCP_WND + TCP_SND_BUF)/TCP_MSS  //能够同时监听的TCP的PCB数目
#define MEMP_NUM_TCP_SEG                    40  　　　//80 能够同时在队列里的TCP的PCB数目
#define MEMP_NUM_REASSDATA                   8       //最大同时等待重装的IP包数目，是整个IP包，不是IP分片
#define MEMP_NUM_ARP_QUEUE                  30       //最大等待回复ARP请求的数目(ARP_QUEUEING有效)
#define MEMP_NUM_IGMP_GROUP                  8       //多播组同时接收数据的最大成员数目(LWIP_IGMP有效)
#define MEMP_NUM_SYS_TIMEOUT                20       //能够同时激活的超时连接数目(NO_SYS==0有戏)
#define MEMP_NUM_NETBUF                     10       //netbufs结构的数目
#define MEMP_NUM_NETCONN                    16       //netconns结构的数目
#define MEMP_NUM_TCPIP_MSG_API              40       //tcpip_msg结构的最大数目，用于callback和API的通讯 by zc
#define MEMP_NUM_TCPIP_MSG_INPKT            40       //tcpip_msg接受数据包结构的最大数目 by zc
#define PBUF_POOL_SIZE                      48       //内存池数量(小内存减小该选项可大大减小内存占用)

//*****************************************************************************
//
// ---------- ARP options ----------
//
//*****************************************************************************
#define LWIP_ARP                            1        //ARP协议允许
#define ARP_TABLE_SIZE                      10  　　　//ARP维护的表项大小
#define ARP_QUEUEING                        1   　　　//硬件地址解析时，将发送数据包计入队列
#define ETHARP_TRUST_IP_MAC                 1   　　　//所有IP数据包都会直接引起ARP table的更新，
                                                     //为0则非表项内IP-MAC关系会引起ARP请求,可以避免IP欺骗，不过会造成延时
#define ETHARP_SUPPORT_VLAN                 0   　　　//非虚拟局域网，为1则仅虚拟局域网通讯有效

//*****************************************************************************
//
// ---------- IP options ----------
//
//*****************************************************************************
#define IP_FORWARD                          0    //不允许不匹配数据包转发,多接口时为1
#define IP_OPTIONS_ALLOWED                  1    //带IP选项数据包允许 为0则丢弃所有IP数据包
#define IP_REASSEMBLY                       1    //允许接收IP包分片包(为0不允许，不能够接收大于MTU的包)
#define IP_FRAG                             1    //允许发送IP包分片包
#define IP_REASS_MAXAGE                     3    //允许接收的最大分段数
#define IP_REASS_MAX_PBUFS                  10   //最大允许存在的IP分片包占用的内存池个数
#define IP_FRAG_USES_STATIC_BUF             1    //IP分片使用静态缓冲区
#define IP_FRAG_MAX_MTU                     1500 //IP分片最大缓冲数量
#define IP_DEFAULT_TTL                      255  //IP数据包最大经历设备数目
#define IP_SOF_BROADCAST                    0    //IP发送广播包过滤
#define IP_SOF_BROADCAST_RECV               0    //IP接收广播包过滤

//*****************************************************************************
//
// ---------- ICMP options ----------
//
//*****************************************************************************
#define LWIP_ICMP                           1    //开启ping包接收/发送
#define ICMP_TTL                            (IP_DEFAULT_TTL) //ping包最大经历设备数目
#define LWIP_BROADCAST_PING                 0    //不回复广播ping包
#define LWIP_MULTICAST_PING                 0    //不回复多播ping包

//*****************************************************************************
//
// ---------- RAW options ----------
//
//*****************************************************************************
#define LWIP_RAW                            0              //无操作系统基于回调函数驱动
#define RAW_TTL                           (IP_DEFAULT_TTL) //应用层数据传输次数(基于IP层的TTL)

//*****************************************************************************
//
// ---------- DHCP options ----------
//
//*****************************************************************************
#define LWIP_DHCP                           0    //动态主机协议配置(为1时)
#define LWIP_AUTOIP                         0    //动态主机IP地址配置(为1时)
#define LWIP_DHCP_AUTOIP_COOP               0    //允许上述两种配置同时存在于1个接口(为1时)
#define LWIP_DHCP_AUTOIP_COOP_TRIES         9    //DHCP分配允许失败次数，失败则使用AUTOUP

//*****************************************************************************
//
// ---------- IGMP options ----------
//
//*****************************************************************************
#define LWIP_IGMP                           0            //LWIP组管理协议

//*****************************************************************************
//
// ---------- DNS options -----------
//
//*****************************************************************************
#define LWIP_DNS                            0     //域名服务器模块(依托UDP协议)
#define DNS_TABLE_SIZE                      4     //域名服务器维护的最大入口数目
#define DNS_MAX_NAME_LENGTH                 256   //域名服务器主机地址最大长度
#define DNS_MAX_SERVERS                     2     //域名服务器最大服务数目
#define DNS_DOES_NAME_CHECK                 1     //查询域名服务器时检测地址名
#define DNS_USES_STATIC_BUF                 1     //域名服务器使用静态地址
#define DNS_MSG_SIZE                        512   //域名服务器接收最大通讯数据长度
#define DNS_LOCAL_HOSTLIST                  0     //在本地维护域名服务器主机-地址表(为1时)
#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC       0     //主机-地址表实时更新(为1时)

//*****************************************************************************
//
// ---------- UDP options ----------
//
//*****************************************************************************
#define LWIP_UDP                               1            //启用UDP协议(snmp协议基于此)
#define LWIP_UDPLITE                           1            //UDP协议启用精简版
#define UDP_TTL                                (IP_DEFAULT_TTL) //UDP数据包传输次数
#define LWIP_NETBUF_RECVINFO                   0            //接收到的数据包除首个外其它不附加目的地址和端口

//*****************************************************************************
//
// ---------- TCP options ----------
//
//*****************************************************************************
#define LWIP_TCP                            1                                  //启用TCP协议(http协议基于此)
#define TCP_TTL                             (IP_DEFAULT_TTL)                   //TCP数据包传输次数
#define TCP_WND                             4*TCP_MSS                          //tcp窗口大小
#define TCP_MAXRTX                          12                                 //最大允许重传TCP数据包数目
#define TCP_SYNMAXRTX                       6                                  //最大允许重传SYN连接包数目
#define TCP_QUEUE_OOSEQ                     (LWIP_TCP)                         //TCP接收队列外数据块排序
#define TCP_MSS                             1460                               //tcp报文最大分段长度
#define TCP_CALCULATE_EFF_SEND_MSS          1                                  //tcp预计发送的分段长度，为1则根据窗口大小分配
#define TCP_SND_BUF                         (8*TCP_MSS)                        //TCP发送缓冲区  zc 7.1
#define TCP_SND_QUEUELEN                    (4*(TCP_SND_BUF/TCP_MSS))          //TCP发送队列长度
#define TCP_SNDLOWAT                        (TCP_SND_BUF/4)                    //TCP可发送数据长度
#define TCP_LISTEN_BACKLOG                  1                                  //TCP多连接允许
#define TCP_DEFAULT_LISTEN_BACKLOG          0xff                               //TCP连接和半连接的总数
#define LWIP_TCP_TIMESTAMPS                 0                                  //TCP时间戳选项
#define TCP_WND_UPDATE_THRESHOLD            (TCP_WND / 4)                      //TCP窗口更新阈值


//TCP定时器设置选项
#define  TCP_MSL                             10000UL                            //TCP连接存在时间 单位ms
#define  TCP_FIN_WAIT_TIMEOUT                20000UL                            //FIN等到ACK应答时间 单位ms
#define  TCP_TMR_INTERVAL                    20                                 //TCP定时器计数间隔 20ms

//*****************************************************************************
//
// ---------- Internal Memory Pool Sizes ----------
//
//*****************************************************************************
 #define  PBUF_LINK_HLEN                 14           //为处理以太网头申请的包长度(本地MAC地址+远端MAC地址+协议类型) 6+6+2
 #define  PBUF_POOL_BUFSIZE             256           //单个内存池长度，要考虑到4字节对齐和最佳大小
 #define  ETH_PAD_SIZE                  0             //以太网填充长度，stm32设置为0 根据发送包判断

//关系不大.. 默认即可

//*****************************************************************************
//
// ---------- Sequential layer options ----------
//
//*****************************************************************************
#define LWIP_TCPIP_CORE_LOCKING           0           //?
#define LWIP_NETCONN                      1           //应用层使用NETCONN相关函数

//*****************************************************************************
//
// ---------- Socket Options ----------
//
//*****************************************************************************
#define LWIP_SOCKET                       0               //关闭SOCKET通讯
//#define LWIP_COMPAT_SOCKETS             1
//#define LWIP_POSIX_SOCKETS_IO_NAMES     1
#define LWIP_SO_RCVTIMEO                  0               //conn->acceptmbox/recvmbox接收有超时限制,超时后自动断开连接
//#define LWIP_SO_RCVBUF                  0
//#define SO_REUSE                        0

//*****************************************************************************
//
// ---------- Statistics options ----------
//
//*****************************************************************************
 #define LWIP_STATS                      0                      //LWIP统计选项

//*****************************************************************************
//
// ---------- checksum options ----------
//
//*****************************************************************************
#define CHECKSUM_GEN_IP                      1                   //IP校验和生成
#define CHECKSUM_GEN_UDP                     1                   //UDP校验和生成
#define CHECKSUM_GEN_TCP                     1                   //TCP校验和生成
#define CHECKSUM_CHECK_IP                    1                   //IP校验和校验
#define CHECKSUM_CHECK_UDP                   1                   //UDP校验和校验
#define CHECKSUM_CHECK_TCP                   1                   //TCP校验和校验

//*****************************************************************************
//
// ---------- TCP KEEPLIVE ----------
//
//*****************************************************************************
#define LWIP_TCP_KEEPALIVE                  1                //tcp保活定时器
#define TCP_KEEPIDLE_DEFAULT                60000            //保活时间 60s
#define TCP_KEEPINTVL_DEFAULT               10000            //保活探测 10s
#define TCP_KEEPCNT_DEFAULT                 9U

//*****************************************************************************
//
// ---------- DEBUG_OPTIONS  ----------
//
//*****************************************************************************
#define DEBUG_OPTIONS                                   0                    //调试代码

#endif
