module screw(){
    circle(d=2.2,$fn=30);
}

module pcb(){
    color("SteelBlue") linear_extrude(height=1) difference(){

            square([28,27.5],center=true);
            translate([0,-12.5]) square([13.5,2.5],center=true);
        translate([28/2-2.1,27.5/2-2.1]) screw();
        translate([-28/2+2.1,-27.5/2+2.1]) screw();
        translate([-28/2+2.1,27.5/2-2.1]) screw();
        translate([28/2-2.1,-27.5/2+2.1]) screw();
        
        translate([2.54/2,23.2/2+0.6]) circle(d=1.02,$fn=30);
        translate([2.54*3/2,23.2/2+0.6]) circle(d=1.02,$fn=30);
        translate([-2.54/2,23.2/2+0.6]) circle(d=1.02,$fn=30);
        translate([-2.54*3/2,23.2/2+0.6]) circle(d=1.02,$fn=30);
        
    }
}

module screen(){
color("Black") linear_extrude(height=1) square([28,19.5],center=true);
}

module cable(){
    color("black") union(){
translate([0,-12,0.9]) rotate([0,90,180])rotate_extrude(angle=180,$fn=200)translate([1,0])rotate([0,0,90]) square([12.5,0.2],center=true);
    
    translate([-6.25,-12,1.8]) cube([12.5,7,0.2]);
    translate([-6.25,-12,-0.2]) cube([12.5,1,0.2]);
    }
}
module pin(){
color("Silver") linear_extrude(height=11.54) square([0.65,0.65],center=true);
color("Black")translate([0,0,3]) linear_extrude(height=2.54) difference(){
        square([2.5,2.5],center=true);
            rotate([0,0,45]) translate([2.54,0]) square([2,2],center=true);
            rotate([0,0,135]) translate([2.54,0]) square([2,2],center=true);
            rotate([0,0,225]) translate([2.54,0]) square([2,2],center=true);
            rotate([0,0,315]) translate([2.54,0]) square([2,2],center=true);
    }
}

module SSD1306(pin=0){
pcb();
translate([0,0,1]) screen();
    cable();
    if (pin==1){
 translate([2.54/2,23.2/2+0.6,3]) rotate([180,0,0]) pin();
 translate([2.54*3/2,23.2/2+0.6,3]) rotate([180,0,0]) pin();
 translate([-2.54/2,23.2/2+0.6,3]) rotate([180,0,0]) pin();
 translate([-2.54*3/2,23.2/2+0.6,3]) rotate([180,0,0])pin();
        }
}


SSD1306(pin=1);