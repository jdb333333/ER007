/*
        HTTP CLIENT FOR RAW LWIP
        (c) 2008-2009 Noyens Kenneth
        PUBLIC VERSION V0.2 16/05/2009

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1 as published by
        the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the
        Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include <stdlib.h>
#include <string.h>
#include "webclient.h"

#include "utils/ustdlib.h"

// Close a PCB(connection)
void hc_clearpcb(struct tcp_pcb *pcb)
{
    if (pcb != NULL)
    {
        // Close the TCP connection
        tcp_close(pcb);
    }
}

// Function that lwip calls for handling recv'd data
err_t hc_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct hc_state *state = arg;
    char * page = NULL;
    struct pbuf * temp_p;
    hc_errormsg errormsg = GEN_ERROR;
    int i;

    if ((err == ERR_OK) && (p != NULL))
    {
        tcp_recved(pcb, p->tot_len);

        // Add payload (p) to state
        temp_p = p;
        while (temp_p != NULL)
        {
            state->RecvData = realloc(state->RecvData, temp_p->len + state->Len + 1);

            // CHECK 'OUT OF MEM'
            if (state->RecvData == NULL)
            {
                // OUT OF MEMORY
                (*state->ReturnPage)(state->Num, OUT_MEM, NULL, 0);
                return(ERR_OK);
            }

            strncpy(state->RecvData + state->Len, temp_p->payload, temp_p->len);
            state->RecvData[temp_p->len + state->Len] = '\0';
            state->Len += temp_p->len;

            temp_p = temp_p->next;
        }

        // Removing payloads

        while (p != NULL)
        {
            temp_p = p->next;
            pbuf_free(p);
            p = temp_p;
        }

    }

    // NULL packet == CONNECTION IS CLOSED(by remote host)
    else if ((err == ERR_OK) && (p == NULL))
    {
        // Simple code for checking 200 OK
        for (i=0; i < state->Len; i++)
        {
            if (errormsg == GEN_ERROR)
            {
                // Check for 200 OK
                if ((*(state->RecvData+i) == '2') && (*(state->RecvData+ ++i) == '0') && (*(state->RecvData+ ++i) == '0')) errormsg = OK;
                if (*(state->RecvData+i) == '\n') errormsg = NOT_FOUND;
            } else
            {
                // Remove headers
                if ((*(state->RecvData+i) == '\r') && (*(state->RecvData+ ++i) == '\n') && (*(state->RecvData+ ++i) == '\r') && (*(state->RecvData + ++i) == '\n'))
                {
                    i++;
                    page = malloc(strlen(state->RecvData+i));
                    strcpy(page, state->RecvData+i);
                    break;
                }
            }
        }

        if (errormsg == OK)
        {
            // Put recv data to ---> p->ReturnPage
            (*state->ReturnPage)(state->Num, OK, page, state->Len);
        } else
        {
            // 200 OK not found Return NOT_FOUND (WARNING: NOT_FOUND COULD ALSO BE 5xx SERVER ERROR, ...)
            (*state->ReturnPage)(state->Num, errormsg, NULL, 0);
        }

        // Clear the PCB
        hc_clearpcb(pcb);

        // free the memory containing state
        free(state->RecvData);
        free(state);
    }

    return(ERR_OK);
}

// Function that lwip calls when there is an error
static void hc_error(void *arg, err_t err)
{
    struct hc_state *state = arg;
    // pcb already deallocated

    // Call return function
    // TO-DO: Check err_t err for out_mem, ...
    (*state->ReturnPage)(state->Num, GEN_ERROR, NULL, 0);

    free(state->RecvData);
    free(state->PostVars);
    free(state->Page);
    free(state);
}

// Function that lwip calls when the connection is idle
// Here we can kill connections that have stayed idle for too long
static err_t hc_poll(void *arg, struct tcp_pcb *pcb)
{
    struct hc_state *state = arg;

    state->ConnectionTimeout++;
    if (state->ConnectionTimeout > 20)
    {
        // Close the connection
        tcp_abort(pcb);

        // Give err msg to callback function
        // Call return function
        (*state->ReturnPage)(state->Num, TIMEOUT, NULL, 0);
    }

    return(ERR_OK);
}

// lwip calls this function when the remote host has successfully received data (ack)
static err_t hc_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct hc_state *state = arg;

    // Reset connection timeout
    state->ConnectionTimeout = 0;

    return(ERR_OK);
}

// lwip calls this function when the connection is established
static err_t hc_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct hc_state *state = arg;
    char  * headers;

    // error?
    if (err != ERR_OK)
    {
        hc_clearpcb(pcb);

        // Call return function
        (*state->ReturnPage)(state->Num, GEN_ERROR, NULL, 0);

        // Free wc state
        free(state->RecvData);
        free(state);

        return(ERR_OK);
    }

    // Define Headers
    if (state->PostVars == NULL)
    {
        // GET headers (without page)(+ \0) = 19
        headers = malloc(19 + strlen(state->Page));
        xprintf(headers,"GET /%s HTTP/1.0\r\n\r\n", state->Page);
    } else
    {
        // POST headers (without PostVars or Page)(+ \0) = 91
        // Content-length: %d <==                                                    ??? (max 10)
        headers = malloc(91 + strlen(state->PostVars) + strlen(state->Page) + 10);
        xprintf(headers, "POST /%s HTTP/1.0\r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: %d\r\n\r\n%s\r\n\r\n", state->Page, strlen(state->PostVars), state->PostVars);
    }

    // Check if we are nut running out of memory
    if (headers == NULL)
    {
        hc_clearpcb(pcb);

        // Call return function
        (*state->ReturnPage)(state->Num, OUT_MEM, NULL, 0);

        // Free wc state
        free(state->RecvData);
        free(state);

        return(ERR_OK);
    }

    // Setup the TCP receive function
    tcp_recv(pcb, hc_recv);

    // Setup the TCP error function
    tcp_err(pcb, hc_error);

    // Setup the TCP polling function/interval         //TCP_POLL IS NOT CORRECT DEFINED @ DOC!!!
    tcp_poll(pcb, hc_poll, 10);

    // Setup the TCP sent callback function
    tcp_sent(pcb, hc_sent);

    // Send data
    tcp_write(pcb, headers, strlen(headers), 1);
    tcp_output(pcb);

    // remove headers
    free(headers);
    free(state->PostVars);                        // postvars are send, so we don't need them anymore
    free(state->Page);                                    // page is requested, so we don't need it anymore

    return(ERR_OK);
}


// Public function for request a webpage (REMOTEIP, ...
int hc_open(struct ip_addr remoteIP, char *Page, char *PostVars, void (* returnpage)(u8_t, hc_errormsg, char *, u16_t))
{
    struct tcp_pcb *pcb = NULL;
    struct hc_state *state;
    static u8_t num = 0;
    // local port
    u16_t port= 4545;

    // Get a place for a new webclient state in the memory
    state = malloc(sizeof(struct hc_state));

    // Create a new PCB (PROTOCOL CONTROL BLOCK)
    pcb = tcp_new();
    if (pcb == NULL || state == NULL)
    {
        //xprintf("hc_open: Not enough memory for pcb or state\n");
        //Not enough memory
        return 0;
    }

    // Define webclient state vars
    num++;
    state->Num = num;
    state->RecvData = NULL;
    state->ConnectionTimeout = 0;
    state->Len = 0;
    state->ReturnPage = returnpage;

    // Make place for PostVars & Page
    if (PostVars != NULL) state->PostVars = malloc(strlen(PostVars) +1);
    state->Page = malloc(strlen(Page) +1);

    // Check for "out of memory"
    if (state->Page == NULL || (state->PostVars == NULL && PostVars != NULL))
    {
        free(state->Page);
        free(state->PostVars);
        free(state);
        tcp_close(pcb);
        return 0;
    }
    // Place allocated copy data
    strcpy(state->Page, Page);
    if (PostVars != NULL) strcpy(state->PostVars, PostVars);

    // Bind to local IP & local port
    while (tcp_bind(pcb, IP_ADDR_ANY, port) != ERR_OK)
    {
        // Local port in use, use port+1
        port++;
    }

    // Use conn -> argument(s)
    tcp_arg(pcb, state);

    // Open connect (SEND SYN)
    tcp_connect(pcb, &remoteIP, 80, hc_connected);

    return num;
}


//======================================================================
//HTTP请求：
#include   
#include   
#include   
#include   
// Http请求内容
static const char send_data[] =
"GET /v1.0/device/13855/sensor/22988/datapoints HTTP/1.1\r\n"
"U-ApiKey:[your apikey]\r\n"
"Host: api.yeelink.net\r\n\r\n";
void tcpclient(const char* host_name, int port)
{
    (void)port;

    char *recv_data;
    int sock, bytes_received;

    struct hostent *yeelink_host;
    struct in_addr yeelink_ipaddr;
    struct sockaddr_in yeelink_sockaddr;

    recv_data = rt_malloc(1024);
    if (recv_data == RT_NULL)
    {
        xprintf("No memory\r\n");
        return;
    }
    // 第一步 DNS地址解析
    xprintf("calling gethostbyname with: %s\r\n", host_name);
    yeelink_host = gethostbyname(host_name);
    yeelink_ipaddr.s_addr = *(unsigned long *) yeelink_host->h_addr_list[0];
    xprintf("Yeelink IP Address:%s\r\n" , inet_ntoa(yeelink_ipaddr));


    yeelink_sockaddr.sin_family = AF_INET;
    yeelink_sockaddr.sin_port = htons(80);
    yeelink_sockaddr.sin_addr = yeelink_ipaddr;
    rt_memset(&(yeelink_sockaddr.sin_zero), 0, sizeof(yeelink_sockaddr.sin_zero));

    while (1)
    {
        // 第二步 创建套接字
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            xprintf("Socket error\n");
            rt_free(recv_data);
            return;
        }

        // 第三步 连接yeelink
        if (connect(sock, (struct sockaddr *)&yeelink_sockaddr, sizeof(struct sockaddr)) == -1)
        {
            xprintf("Connect fail!\n");
            lwip_close(sock);
            rt_free(recv_data);
            return;
        }

        // 第4步 发送Http请求
        send(sock,send_data,strlen(send_data), 0);

        // 第5步 获得Http响应
        bytes_received = recv(sock, recv_data, 1024 - 1, 0);
        recv_data[bytes_received] = '\0';

        // 响应内容为 {"timestamp":"2013-11-19T08:50:11","value":1}
        // 截取“value”之后的内容
        char* actuator_info = rt_strstr( recv_data , "\"value\"");
        int offset = rt_strlen("\"value\":");
        actuator_status = *(actuator_info + offset);
        xprintf("actuator status :%c\r\n",actuator_status);

        // 获得开关状态，并设置LED指示灯
        char actuator_status;
        (actuator_status == '1')?rt_hw_led_on(0):rt_hw_led_off(0);

        rt_memset(recv_data , 0 , sizeof(recv_data));

        // 关闭套接字
        closesocket(sock);

        // 延时5S之后重新连接
        rt_thread_delay( RT_TICK_PER_SECOND * 5 );
    }
}

/*===================================================================================
This ended up being pretty simple to implement, forgot to update this question.
I pretty much followed the instructions given on this site, which is the Raw/TCP
'documentation'.Basically, The HTTP request is encoded in TCP packets, so to send
data to my PHP server, I sent an HTTP request using TCP packets (lwIP does all the work).

The HTTP packet I want to send looks like this:
    HEAD /process.php?data1=12&data2=5 HTTP/1.0
    Host: mywebsite.com

To "translate" this to text which is understood by an HTTP server, you have to add "\r\n"
carriage return/newline in your code. So it looks like this:

char *string = "HEAD /process.php?data1=12&data2=5 HTTP/1.0\r\nHost: mywebsite.com\r\n\r\n ";

Note that the end has two lots of "\r\n"

You can use GET or HEAD, but because I didn't care about HTML site my PHP server returned,
I used HEAD (it returns a 200 OK on success, or a different code on failure).
The lwIP raw/tcp works on callbacks. You basically set up all the callback functions,
then push the data you want to a TCP buffer (in this case, the TCP string specified above),
and then you tell lwIP to send the packet.

Function to set up a TCP connection (this function is directly called by my application
every time I want to send a TCP packet):
=====================================================================*/
void tcp_setup(void)
{
    uint32_t data = 0xdeadbeef;

    /* create an ip */
    struct ip_addr ip;
    IP4_ADDR(&ip, 110,777,888,999);    //IP of my PHP server

    /* create the control block */
    testpcb = tcp_new();    //testpcb is a global struct tcp_pcb
                            // as defined by lwIP


    /* dummy data to pass to callbacks*/

    tcp_arg(testpcb, &data);

    /* register callbacks with the pcb */

    tcp_err(testpcb, tcpErrorHandler);
    tcp_recv(testpcb, tcpRecvCallback);
    tcp_sent(testpcb, tcpSendCallback);

    /* now connect */
    tcp_connect(testpcb, &ip, 80, connectCallback);

}

//Once a connection to my PHP server is established, the 'connectCallback' function is called by lwIP:

/* connection established callback, err is unused and only return 0 */
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    xprintf("Connection Established.\n");
    xprintf("Now sending a packet\n");
    tcp_send_packet();
    return 0;
}

//This function calls the actual function tcp_send_packet() which sends the HTTP request, as follows:

uint32_t tcp_send_packet(void)
{
    char *string = "HEAD /process.php?data1=12&data2=5 HTTP/1.0\r\nHost: mywebsite.com\r\n\r\n ";
    uint32_t len = strlen(string);

    /* push to buffer */
    error = tcp_write(testpcb, string, strlen(string), TCP_WRITE_FLAG_COPY);

    if (error)
    {
        xprintf("ERROR: Code: %d (tcp_send_packet :: tcp_write)\n", error);
        return 1;
    }

    /* now send */
    error = tcp_output(testpcb);
    if (error)
    {
        xprintf("ERROR: Code: %d (tcp_send_packet :: tcp_output)\n", error);
        return 1;
    }
    return 0;
}

//Once the TCP packet has been sent (this is all need if you want to "hope for the best"
//and don't care if the data actually sent), the PHP server return a TCP packet (with a 200 OK, etc.
//and the HTML code if you used GET instead of HEAD). This code can be read and verified in the following code:

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    xprintf("Data recieved.\n");
    if (p == NULL)
    {
        xprintf("The remote host closed the connection.\n");
        xprintf("Now I'm closing the connection.\n");
        tcp_close_con();
        return ERR_ABRT;
    } else
    {
        xprintf("Number of pbufs %d\n", pbuf_clen(p));
        xprintf("Contents of pbuf %s\n", (char *)p->payload);
    }

    return 0;
}


