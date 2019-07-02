# Port-of-SimAVR
Like many MicroControllers available namely 8051, 8031 etc which basically uses C(embedded C) code, we also have others like Atmel
ATMega 1280 mcu(Micro Controller Unit) which uses AVR codes. Like other programming, Programming in AVR also needs knowledge about
its syntax but its like impractical to say that ‘errors won’t be arised’. So, as a debugger and also many a times simulation of such
programs in the absenceof actual hardware, so here comes the need of some debugging tool or software which can help us to do so for
which we have “SimAVR”.<br>
SimAVR comes as an github repository from which it needs to be cloned first and then installation of a software named GTKWave is required
for simulation. As we know, it is not really practical and possible to have all necessary machine requirements, memory space, time and 
most importantly patience to download, install and follow their “ReadMe”. In this modern era of mobile phones, which are much more handy
than laptops or systems. Thus, we found the need of an hour and so we here present Port of SimAVR to make it accessible through web on 
every device(even on, thus, phones) at no cost of installation memory, time or patience.
<br><br>

<h2>COMPILING</h2>
1. Install emsripten SDK from its official website, https://emscripten.org/docs/getting_started/downloads.html .<br>
2. Clone our repository.<br>
3. Change directory to "CodeBase" and configure cmake file and make necessary builds to take place.<br>
>><b>cd CodeBase<br></b>
>><b>emconfigure cmake</b><br><br>
4. Update link.txt file to make it generate js without wasm, passing necessary flags to emcc command as:<br>
<b>-s ASM_JS=1 -s WASM=0 -s FORCE_FILESYSTEM=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0  -s 
EXTRA_EXPORTED_RUNTIME_METHODS=’[“ccall”,”cwrap”,”ALLOC_NORMAL”]’</b><br>
5. Now run the build file by command:<br>
>><b>emmake make</b><br><br>
6. Replace the js file generated, namely, "simavr.js", into the "Front_End" directory.<br>
7. Run Python Server in current directory i.e "Front_End", to serve it locally on your machine.<br><br>


<h2>How to use?</h2>
1. This is Port of Atmel ATMega1280 mcu(Micro Control Unit) which has 11 in total Ports(PA, PB, ...PK, excuding PI) for connection acc.
to its Pin-Outs.
Each Port having 8 pins and thus its LEDs representation.<br>
2. To convert an elf file into hex file, use command:<Br>
>><b>avr-objcopy -j .text -j .data -O ihex main.elf main.hex</b><br><br>
3. To increase the simulation speed on the cost of higher CPU utilization, slider can be used.<br>
4. To stop the simulation process at some point of time, "Abort Simulation" can be used.<br>
5. To Debug AVR codes using it, upload a generated file in .hex format(generating from AVR codes).<br>
The simulation is being shown in form of LEDs which is the first and direct visual sign of debugging, as comparing the results with
as expected and shown.
6. Ouput Terminal can be used to debug as the second approach i.e the precise and accurate details which highlights the points which
may help to debug namely no. of machine cycles taken by instructions, time delay, time taken in execution of each cycle etc.
(depending upon hardware being used). Better interpretation out of it, may validate the input in just one go!
<br><br>

<h2><b>System Specifications(While Porting)</b></h2>
Working Machine: Ubuntu 18.04<br>
Emscripten version used: emcc 1.38.36<br><br>

<h2>LIMITATIONS</h2>
<ol>
<li>The port supports especially ATMega 1280 mcu, thus other versions of mcu are not supported.
<li>Rendering of LCD screen i.e Graphic representation is not available. However, mostly information is being printed on terminal at this stage to simplify debug process.
<li>The timing is fixed as described for the 5V-versions.
<li>Only .hex file format is acceptable. However, command has been provided to convert an elf file into .hex .</ol><br>

<h2>CONTRIBUTING</h2>
We welcome the patches. Please submit your changes by Pull requests.<br>

<h2>ORIGINAL</h2>
As this is a port, the original, official SimAVR can be found at https://github.com/buserror/simavr .<br>
