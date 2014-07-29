/*
*   Declaration of handlers public interface
*/

#ifndef HANDLERS_H__
#define HANDLERS_H__

void setAlarmhandler(void);
void setIOhandler(void);
void on_input(int);
void setup_aio_buffer(void);
void setup_aio_read(void);

#endif //HANDLERS_H__

