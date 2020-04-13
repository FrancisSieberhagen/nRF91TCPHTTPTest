/*
 *
 * TCP HTTP Test
 *
 */

#include <zephyr.h>
#include <bsd.h>
#include <net/socket.h>
#include <modem/lte_lc.h>

#include <drivers/gpio.h>
#include <stdio.h>
#include <unistd.h>

#include "cJSON.h"

#define LED_PORT	DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED1 DT_GPIO_LEDS_LED0_GPIOS_PIN
#define LED2 DT_GPIO_LEDS_LED1_GPIOS_PIN
#define LED3 DT_GPIO_LEDS_LED2_GPIOS_PIN
#define LED4 DT_GPIO_LEDS_LED3_GPIOS_PIN


#define HTTP_HEAD_1   \
    "GET http://worldtimeapi.org/api/timezone/Africa/Johannesburg HTTP/1.1\r\n" \
    "Host: worldtimeapi.org\r\n" \
    "User-Agent: curl/7.64.1\r\n" \
    "Accept: */*\r\n" \
    "Proxy-Connection: Keep-Alive\r\n\r\n"

static int server_socket;
static struct sockaddr_storage server;

LOG_MODULE_REGISTER(app, CONFIG_TEST1_LOG_LEVEL);

#if defined(CONFIG_BSD_LIBRARY)

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
  printk("bsdlib recoverable error: %u\n", err);
}

/**@brief Irrecoverable BSD library error. */
void bsd_irrecoverable_error_handler(uint32_t err)
{
  printk("bsdlib irrecoverable error: %u\n", err);

  __ASSERT_NO_MSG(false);
}

#endif /* defined(CONFIG_BSD_LIBRARY) */

struct device *led_device;

static void init_led()
{

    led_device = device_get_binding(LED_PORT);

    /* Set LED pin as output */
    gpio_pin_configure(led_device, LED1, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED2, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED3, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED4, GPIO_OUTPUT);

}

static void led_on(char led)
{
    gpio_pin_set(led_device, led, 1);
}
static void led_off(char led)
{
    gpio_pin_set(led_device, led, 0);
}


static void init_modem(void)
{
    int err;

     err = lte_lc_init_and_connect();
    __ASSERT(err == 0, "ERROR: LTE link init and connect %d\n", err);

    err = lte_lc_psm_req(false);
    __ASSERT(err == 0, "ERROR: psm %d\n", err);

     err = lte_lc_edrx_req(false);
    __ASSERT(err == 0, "ERROR: edrx %d\n", err);

}

static int tcp_ip_resolve(void)
{
    struct addrinfo *addrinfo;

    struct addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM};

    char ipv4_addr[NET_IPV4_ADDR_LEN];

    if (getaddrinfo(CONFIG_SERVER_HOST, NULL, &hints, &addrinfo) != 0)
    {
        LOG_ERR("ERROR: getaddrinfo failed\n");
        return -EIO;
    }

    if (addrinfo == NULL)
    {
        LOG_ERR("ERROR: Address not found\n");
        return -ENOENT;
    }

    struct sockaddr_in *server_ipv4 = ((struct sockaddr_in *)&server);

    server_ipv4->sin_addr.s_addr = ((struct sockaddr_in *)addrinfo->ai_addr)->sin_addr.s_addr;
    server_ipv4->sin_family = AF_INET;
    server_ipv4->sin_port = htons(CONFIG_SERVER_PORT);

    inet_ntop(AF_INET, &server_ipv4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
    LOG_INF("Server IPv4 Address %s\n", log_strdup(ipv4_addr));

    freeaddrinfo(addrinfo);

    return 0;
}

int connect_to_server()
{
    int err;

    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        LOG_ERR("Failed to create CoAP socket: %d.\n", errno);
        return -errno;
    }

    err = connect(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        LOG_ERR("Connect failed : %d\n", errno);
        return -errno;
    }

    return 0;
}

static void action_json_msg(char *msgbuf) {

    /*
     {
       "week_number": 16,
       "utc_offset": "+02:00",
       "utc_datetime": "2020-04-13T07:08:04.066847+00:00",
       "unixtime": 1586761684,
       "timezone": "Africa/Johannesburg",
       "raw_offset": 7200,
       "dst_until": null,
       "dst_offset": 0,
       "dst_from": null,
       "dst": false,
       "day_of_year": 104,
       "day_of_week": 1,
       "datetime": "2020-04-13T09:08:04.066847+02:00",
       "client_ip": "172.104.140.63",
       "abbreviation": "SAST"
     }
     */

    bool found_json = false;

    char *start = "{";
    char *end = "}";

    char *p_msgbuf = msgbuf;

    if (*p_msgbuf == *start)
    {
        p_msgbuf += (strlen(msgbuf) - 1);
        if (*p_msgbuf == *end)
        {
            found_json = true;
        }
    }

    if (found_json == false)
    {
        LOG_ERR("ERROR: No JSON Data found\n");
        return;
    }

    cJSON *monitor_json = cJSON_Parse(msgbuf);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_ERR("ERROR: cJSON Parse : %s\n", error_ptr);
            return;
        }
    }

    cJSON *datetime = cJSON_GetObjectItemCaseSensitive(monitor_json, "datetime");
    if (cJSON_IsString(datetime) && (datetime->valuestring != NULL))
    {
        led_on(LED4);
        LOG_INF("datetime   -> %s", log_strdup(datetime->valuestring));
    }
    cJSON *timezone = cJSON_GetObjectItemCaseSensitive(monitor_json, "timezone");
    if (cJSON_IsString(timezone) && (timezone->valuestring != NULL))
    {
        LOG_INF("timezone   -> %s", log_strdup(timezone->valuestring));
    }
    cJSON *utc_offset = cJSON_GetObjectItemCaseSensitive(monitor_json, "utc_offset");
    if (cJSON_IsString(utc_offset) && (utc_offset->valuestring != NULL))
    {
        LOG_INF("utc_offset -> %s\n", log_strdup(utc_offset->valuestring));
    }

    cJSON_Delete(monitor_json);
}

int find_json_start(char *data) {
    int pos = 0;
    char sub[] = "\r\n\r\n{";

    char *p1, *p2, *p3;
    int i=0,j=0;

    p1 = data;
    p2 = sub;

    for(i = 0; i<strlen(data); i++)
    {
        if(*p1 == *p2)
        {
            p3 = p1;
            for(j = 0;j<strlen(sub);j++)
            {
                if(*p3 == *p2)
                {
                    p3++;p2++;
                }
                else
                    break;
            }
            p2 = sub;
            if(j == strlen(sub))
            {
                pos = i + 5;
                break;
            }
        }
        p1++;
    }
    return pos;
}

char *substring(char *string, int position, int length)
{
    char *p1;
    int c;

    p1 = malloc(length+1);

    if (p1 == NULL)
    {
        LOG_ERR("ERROR: Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (c = 0 ; c < position -1 ; c++)
    {
        string++;
    }

    for (c = 0 ; c < length ; c++)
    {
        *(p1+c) = *string;
        string++;
    }

    *(p1+c) = '\0';

    return p1;
}

int send_tcp_msg()
{
    char msgbuf[500];

    char *p_big_buff;
    int big_buff_pos;
    int recsize = 0;
    int json_start;

    struct timeval  timeout;

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    memset(msgbuf, '\0', sizeof(msgbuf));
    strcpy(msgbuf, HTTP_HEAD_1);

    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    int ret = send(server_socket, msgbuf, strlen(msgbuf), 0);

    LOG_INF("Send packet data [%d] To %s:%d. ret=%d",sizeof(msgbuf), log_strdup(CONFIG_SERVER_HOST), CONFIG_SERVER_PORT, ret);

    big_buff_pos = 0;
    p_big_buff = malloc(1000);

    memset(p_big_buff, '\0', 1000);
    for (;;) {
        memset(msgbuf, '\0', sizeof(msgbuf));
        recsize = recv(server_socket, msgbuf, sizeof msgbuf, 0);
        if (recsize <= 0) {
            break;
        }
        if (big_buff_pos == 0) {
            strcpy(p_big_buff, msgbuf);
        } else
        {
            strcat(p_big_buff, msgbuf);
        }
        memset(msgbuf, '\0', sizeof(msgbuf));
        big_buff_pos += recsize;
    }

    led_on(LED3);

    json_start = find_json_start(p_big_buff);
    char *json_data = substring(p_big_buff, json_start, strlen(p_big_buff));

//    LOG_INF("Recv packet data [%d] start [%d] [%s]\n",strlen(big_buff), data_start, log_strdup(substr));
    LOG_INF("Recv packet data [%d] JSON start @ [%d]",strlen(p_big_buff), json_start);

    action_json_msg(json_data);

    free(json_data);
    free(p_big_buff);

    return 0;
}


void main(void)
{
    init_led();

    LOG_INF("BSD TCP HTTP Test V1.1");
    led_on(LED1);

    LOG_INF("Initializing Modem");
    init_modem();

    int err = tcp_ip_resolve();
    __ASSERT(err == 0, "ERROR: tcp_ip_resolve");


    for (;;) {
        led_off(LED2);
        led_off(LED3);
        led_off(LED4);

        LOG_INF("Connect to %s:%d", log_strdup(CONFIG_SERVER_HOST), CONFIG_SERVER_PORT);
        if (connect_to_server() == 0) {
            led_on(LED2);

            send_tcp_msg();

            close(server_socket);
        } else {
            close(server_socket);
        }
        k_sleep(1000);
    }
}
