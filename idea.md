; This indicates to us that the instructions coming in are going to be to setup the screen
.screen
resolution 800 600
tile_size 10

.entity
scale_factor 3  ; sf of 1 means 1 entity per tile. 
                ; 2 means its the origin tile, and all 
                ; touching neighbors, 3 is 2 with neighbors' neighbors etc
modes {
    "idle": {
        "border": #000000
        "background": #000000
    }
    "active": {
        "border": #00FFFF
        "background": #000000
    }
}

default_mode "idle"


; This indicates that the instructions coming in are now the program to run at runtime
.program

; 
;
;
;




```