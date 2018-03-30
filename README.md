# RCOM_Lab1

This project consisted of the development and testing of a **data link protocol** as the first work of the Computer Networks curricular unit. For the data transmission was used an **asynchronous serial port**. The code must be prepared to identify, and handle cases of error, whether they refer to data or to communication failures. The project was successfully completed.

In this project we use an **RS232** and we implement the **Stop and Wait ARQ Protocol**.

## Execution
Get an asynchronous serial port that connects two computers, choose a file that you want to be transmitted and then wright the following commands in the terminal:
```
git clone https://github.com/BaltasarAroso/RCOM_Lab1.git
make
./transfer /dev/ttsy0 <file_to_be_transmitted>
```

then define the mode, RX - Receiver or TX - Transmitter and that's it, the program will do the rest.
