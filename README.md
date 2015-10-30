dynamic-context-decision-making
===============================
Mike Shvartsman. 

Library for building of models of decision making with dynamic context. If you use it, you should email me at ms44@princeton.edu! Not only would it make me happy, but it would mean I can tell you when new and exciting improvements happen. 

Installing:
---------

You will need cmake and a compiler that plays nicely with c++11. On OSX I highly recommend getting a c++11-friendly GCC via http://hpc.sourceforge.net/, and cmake via homebrew (follow instructions at http://brew.sh/, then `brew install cmake`). Then:

```
mkdir bin
cd bin
cmake ..
make cddm
```

The `cddm` target will compile a shared library libcddm.dylib (or hopefully the equivalent on your platform). To use the library you want to import cddm_main.h, and add the library to your library paths. Right now everything is very fragile so watch your paths. 

If your c++-11 friendly compiler is not your default then you will need to pass that into cmake -- for example, in my system:

```
...
CXX=g++-4.9 cmake ..
...
```

I also personally use ninja instead of make, because I find it faster. To do that you would install ninja (`brew install ninja`) and pass that to `cmake` with the `-G` flag (as in `cmake -G Ninja`), and then use `ninja` instead of `make`. 

Unit tests: 
--------------
I use the Catch unit-testing framework. To build all the tests (235 and counting!), you build the `catch_main` target and run it: 
```
make catch_main
./catch_main
```


Building documentation: 
--------------
`make doc` builds doxygen documentation in html and latex format. Right now latex gets confused by the out-of-source doc build so the TOC is broken, but html is the preferred way to go anyway. 

Example tasks: 
--------------
Some examples task implementations exist in `examples/`. There are also compile targets for them: `flanker_batch`, `axcpt_batch` , `flanker_trace` and `axcpt_trace`, and an `examples` target that builds all of them. Both tasks are those used in the NIPS submission. 

The `trace` variants are meant for generating full random walk trajectory traces for visualization and testing: they output a big pile of CSVs. `R/belief_visualizer.R` has some code for visualizing AX-CPT, but it is not well documented or maintained. `R/nips2015_plots.R` has code for generating the NIPS simulation figures, assuming you have built the `examples` target. Simplest way to run it is to navigate to the `R/` directory and call `Rscript nips2015plots.R`. 

The `event` variants have a little less detail: no full trajectories, but everything else that is in `trace`: individual RTs and accuracies for each trial, as well as other things like the time when the inference started/stopped on each trial. 

The `batch` variants are meant to be fast for cluster execution and work as listeners: they wait for a row of input parameters, output results, and then wait for another row of parameters. A newline exits, and `#` is a comment character (mostly implemented for undocumented config file functionality). Parameter format is `key=value,key=value`. All the parameters supported are under the relevant `*_runner.cpp` files under examples. For one-and-done usage you can go `echo "par1=val1,par2=val2" | ./axcpt_batch` or just `echo "#" | ./axcpt_batch` to run with defaults. 

The interfaces are all a bit hackish, sorry.

Useful parameters to pass into the examples: 

  * `decisionThreshold` should sanely be between 0.5 and 1 (not inclusive), and defines the threshold in posterior probability space for the random walk. Realistic values are in the .9s. 
  * `contextNoise` is a scalar, isomorphic to sample rate on context sampling. Values in the 2-5 range give reasonable RTs with actual sample rate fixed at 10ms. 
  * `targetNoise` is a scalar, isomorphic to sample rate on target sampling. Values in the 2-5 range give reasonable RTs with actual sample rate fixed at 10ms. 
  * `maxTrials` is the number of trials to run. 1000 is enough to exclude bad points, 5000-10000 or more is more reasonable to tease apart things at the top. The reason it is `maxTrials` and not just nTrials is because there are eventual plans to enable early stopping based on monitoring the current estimate variance. Not implemented yet, though. 
