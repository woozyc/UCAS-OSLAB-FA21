#include <net.h>
#include <os/string.h>
#include <screen.h>
#include <emacps/xemacps_example.h>
#include <emacps/xemacps.h>

#include <os/sched.h>
#include <os/mm.h>

EthernetFrame rx_buffers[RXBD_CNT];
EthernetFrame tx_buffer;
uint32_t rx_len[RXBD_CNT];

int net_poll_mode = 1;

volatile int rx_curr = 0, rx_tail = 0;

long do_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength)
{
    // TO DO: 
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    // maybe you need to call drivers' receive function multiple times ?
    int rest_num = num_packet;
    int recv_num;
    while(rest_num > 0){
    	recv_num = (rest_num > 32) ? 32 : rest_num;
    	
        EmacPsRecv(&EmacPsInstance, (EthernetFrame *)kva2pa((uintptr_t)rx_buffers), recv_num); 
        EmacPsWaitRecv(&EmacPsInstance, recv_num, rx_len);
        
        for (int i = 0; i < recv_num; i++){
            kmemcpy((uint8_t *)addr, (uint8_t *)(rx_buffers + i), rx_len[i]);
            addr += rx_len[i];
            *frLength = rx_len[i];
            frLength++;
        }
        rest_num -= recv_num;
    }
    return 1;
}

void do_net_send(uintptr_t addr, size_t length)
{
    // TO DO:
    // send 1 packet
    kmemcpy((uint8_t *)(&tx_buffer), (uint8_t *)addr, length);
    EmacPsSend(&EmacPsInstance, (EthernetFrame *)kva2pa((uintptr_t)&tx_buffer), length);
    EmacPsWaitSend(&EmacPsInstance);
}

void do_net_irq_mode(int mode)
{
    // TO DO:
    // turn on/off network driver's interrupt mode
    if(mode){
	    XEmacPs_IntEnable(&EmacPsInstance,
     					  (XEMACPS_IXR_TX_ERR_MASK | XEMACPS_IXR_RX_ERR_MASK |
     					  (u32)XEMACPS_IXR_FRAMERX_MASK | (u32)XEMACPS_IXR_TXCOMPL_MASK));
	}else{
   		XEmacPs_IntDisable(&EmacPsInstance,
     					   (XEMACPS_IXR_TX_ERR_MASK | XEMACPS_IXR_RX_ERR_MASK |
     					   (u32)XEMACPS_IXR_FRAMERX_MASK | (u32)XEMACPS_IXR_TXCOMPL_MASK));
	}
	net_poll_mode = mode;
}
