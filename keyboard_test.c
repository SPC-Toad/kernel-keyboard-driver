#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define KEYBOARD_TEST _IOR(0, 7, char) // We are sending a character down to device driver. CMD = KEYBOARD_TEST

int fd;   

char my_getchar_user(void) {
  unsigned char ch;
  // Use ioctl to request the next character from the kernel module
  if (ioctl(fd, KEYBOARD_TEST, &ch) < 0) {
    perror("ERROR: KEYBOARD_TEST ioctl() failed");
    return '\0'; // Return null character on failure
  }

  return ch;
} 

int main () {
  unsigned char c;
  unsigned char ctrlNext;
  char ascii;

  char replayBuffer[1000];
  int replayBufferLen = 0;

  unsigned char ctrlPress;
  unsigned char ctrlPressNext;

  unsigned char shiftPressNext;

  int ctrlPressed = 0;
  int shiftPressed = 0;

  static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  fd = open("/proc/keyboard_test", O_RDONLY);
  if (fd < 0) {
    perror("Failed to open /proc/keyboard_test\n");
    return 1;
  }


  while (1) {
    // Get character
    c = my_getchar_user();
    /*
      If you first press ctrl-r (start record) you will then record all keystrokes entered into a buffer. 
      These do not appear on the screen until a subsequent ctrl-p (playback) is encountered. 
      Alternatively, If ctrl-N is entered (ctrl along with capital N), 
      then all input characters are played back when ctrl-p is entered, 
      but the played back string replaces newlines with space characters. 
      You do not need to worry about ctrl or other modifier sequences embedded in your record buffer.
    */
    if (c == 0x1d) { // Check if CTRL is pressed
      ctrlPressed = 1;
      ctrlNext = my_getchar_user();
      if (ctrlNext == 0x13) {  // Check if it's 'R' key (Ctrl + R)
        ctrlPressed = 0;
        replayBufferLen = 0;  // Reset buffer
        while (1) {
          c = my_getchar_user();
          if (c == 0x1d) { // if the ctrl is pressed
            ctrlPressed = 1;
            ctrlPressNext = my_getchar_user();
            if (ctrlPressNext == 0x19) { // Check if it's 'P' key (Ctrl + P)
              printf("\nReplaying:\n%s\n", replayBuffer);
              ctrlPressed = 0;
              break;
            }
            else if (ctrlPressNext == 0x9d) {
              ctrlPressed = 0;
              continue;
            }
          } 
          else if (replayBufferLen < sizeof(replayBuffer) - 1) { // Prevent overflow
            ascii = scancode[(int) c];
            if (ascii) {
              replayBuffer[replayBufferLen++] = ascii;
              replayBuffer[replayBufferLen] = '\0'; // Null-terminate buffer
            }
          }
        }
      } 
      if (ctrlNext == 0x9d) { // if it was released
        ctrlPressed = 0;
        continue;
      }
    }

    else if (c == 0x2a || c == 0x36) { // 0x2a (left Shift), 0x36 (right Shift)
      shiftPressed = 1;
      while(shiftPressed) {
        shiftPressNext = my_getchar_user();
        if (shiftPressNext == 0x2a || shiftPressNext == 0x36) { // 0xaa (left Shift release), 0xb6 (right Shift release)
          shiftPressed = 0;
        }
        else {
          ascii = scancode[(int) shiftPressNext];
          if (ascii >= 'a' && ascii <= 'z') {
            ascii -= 32; // Convert lowercase to uppercase using -32
            printf("%c", ascii);
            fflush(stdout);
            // shiftPressed = 0;
          }
        }
      }
    }


    // Handle backspace
    else if (c == 0x0e) {
      printf("\b \b");
      fflush(stdout);
      continue;
    }

    // Regular character input
    else {
      ascii = scancode[(int) c];
      printf("%c", ascii);
      fflush(stdout);
    }
  }
  
  close(fd);
  return 0;
}