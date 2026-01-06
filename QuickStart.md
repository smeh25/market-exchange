# Build and Run Instructions

Run this command from the root directory to build:

```bash
g++ -g -std=c++17 -Wall \
src/*.cpp \
-I./include \
-I$(brew --prefix cppzmq)/include \
-I$(brew --prefix zeromq)/include \
-L$(brew --prefix zeromq)/lib \
-lzmq -lpthread \
-o exchange_core
```

Then run the following to run it:

```bash
./exchange_core
```

After this is complete you should see a message outputted to the terminal that says

<pre>
--- Initializing Market Exchange Core ---
InputStream initialized. In:5555
InputStream: Start listening for orders...
Output socket connected. Out:5556
OrderGenerator thread running
Output socket connected. Out:5556
[CORE] Exchange is LIVE. Waiting for orders...
</pre>

Note if you update the number of threads for the order generators, the number of "Output socket connected" and "OrderGenerator thread running" messages will change accordingly. By default it is set to 8. 

Following this navigate to a new terminal and run the python test scripts.