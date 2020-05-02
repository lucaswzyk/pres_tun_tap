/* Simple example for working with TUN / TAP
 * Most of the code is from https://backreference.org/2010/03/26/tuntap-interface-tutorial/
 * (thanks for that, Waldner)
 * 
 * Slightly adjusted, so it would actually run on my system
 * 
 * To "make this show something", open one console as root / with CAP_NET_ADMIN capability 
 * or use sudo in front of every instruction. Make sure to have openvpn installed and run:
 * 
 * # openvpn --mktun --dev tun77 --user *your_user*
 * Sat May  2 13:30:23 2020 TUN/TAP device tun77 opened
 * Sat May  2 13:30:23 2020 Persist state set to: ON
 * # ip link set tun77 up
 * # ip addr add 10.0.0.1/24 dev tun77
 * 
 * On another console (as *your_user*), run this example:
 * # ./example.out
 * 
 * On the first console, run:
 * # ping 10.0.0.2
 *
 * You should then see something like this in the second console:
 * Read 84 bytes from device tun77
 * Read 84 bytes from device tun77
 * Read 84 bytes from device tun77
 * 
 * author: lucaswzyk
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include </usr/include/linux/if.h>
#include </usr/include/linux/if_tun.h>

/* can be used to 
 * - create a TUN or TAP (needs root / sudo / CAP_NET_ADMIN capability)
 * - attach to an existing TUN or TAP 
 */
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}


/* uses tun_alloc to create / attach to tun77 and reads from it
 * instructions how to "make this show something" at the top
 */
int main() {
  char tun_name[IFNAMSIZ];
  
  /* Connect to the device */
  strcpy(tun_name, "tun77");
  int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);  /* tun interface */

  if(tun_fd < 0){
    perror("Allocating interface");
    exit(1);
  }

  ssize_t nread;
  unsigned char buffer[1500];
  /* Now read data coming from the kernel */
  while(1) {
    /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
    nread = read(tun_fd,buffer,sizeof(buffer));
    if(nread < 0) {
      perror("Reading from interface");
      close(tun_fd);
      exit(1);
    }

    /* Do whatever with the data */
    printf("Read %d bytes from device %s\n", nread, tun_name);
  }
}
