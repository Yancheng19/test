# eTof digi/calib/hit maker

### Usage
  - use *run.sh* to generate .event.root files that contain StETofDigi objects (you will need access rights to .daq files)
  - use *root4star -l 'AnalyzeEvent.C( inputDirectory, inputFile )'* to analyze the event files and read the StETofDigis 
  
  - use *fast-offline.sh* to run the fast-offline chain and produce MuDSTs with StETofDigis 
