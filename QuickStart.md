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
InputStream initialized. In:5555 Out:5556
[CORE] Exchange is LIVE. Waiting for orders...
</pre>

Following this navigate to a new terminal and run the python test scripts.