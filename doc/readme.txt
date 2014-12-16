* BGL
The software was developed with BGL 1.25.1. However, newer versions will also
work.

* Graphviz
For graph visualization , you will need to download the Graphviz utilities at
www.graphviz.org.

* How to run atpg.exe
1) View the sample input graph "Not2.dot"
   - Invoke dotty.
   - In the dotty window, use the right-mouse-button to select "load graph".
   - Use the file navigator that appears to select "Not2.dot".
   - In the dotty window, use the right-mouse-button to select "do layout".
     The sample graph should now appear.
2) Execute atpg.exe.exe -atpg -r Not2.dot -w Not2_out.dot
3) View the generated output graph, "Not2_out.dot".
   For reference, my output graph is Not2_ref.dot.

Fig1.bmp is also included to shows a bitmap of both circuits.

* Test folder
This folder contains the sample circuit Not2.dot and some .bat showing
how to run the Debug and Release versions of atpg.exe.

* Work in progress
As I was learning to use Graphviz, I learnt about a tool based on Graphviz
called Doxygen ( www.doxygen.org ). I am just learning to use doxygen, 
for this project, whose html documentation in doc/dox/html is generated 
with "doxygen atpg.dox". Note that most of the code documentation has
not been modified to work better with doxygen, but the general output
is very encouraging. 


