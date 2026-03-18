## To do ##
- [x] Scale the mesh to a little bigger than the x-y interval
- [x] Position the camera better initially
- [x] Be able to look at it from x, y, z perspective by pressing xyz
- [ ] Add `animating` attribute to camera to make it animate between xyz
- [ ] Also be able to move with mouse
- [ ] Make an input .h and .cpp file to handle inputs
- [x] Add better background and lighting
- [x] Add better surface rendering
- [x] Add a heatmap function to render it as a heatmap
- [ ] Pass a function to it and make it so you can call it from Python
- [ ] Create a config header with information such as solid, heat, color, showaxis, showgrid, animate camera etc. 
- [ ] Come up with a good name
- [ ] Write a proper README
- [ ] Make it public on github
- [ ] See what it takes to make it PDE viewer?
- [ ] Make it function of time as well
- [ ] Make a parser 
    WRONG. It is easier if the grid and function is called in Python during numpy, and that the values are then sent into the C++ renderer. Or just send the function object and let it be calculated in C++. 
- [ ] Add a vector field visualizer


NAME IDEAS
- Surfex (Surface explorer)
- Survex (Surface Vertex)
- Surfium (Surface + element)