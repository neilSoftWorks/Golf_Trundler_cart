// --- PART SELECTOR ---
// 0 = Assembly View
// 1 = Top Housing (Blue) - Print Upside Down
// 2 = Bottom Clamp (Orange) - Print Flat
part_to_print = 0;

// --- Dimensions ---
handle_diam = 32;
clamp_width = 30;

// Spacing
extra_spacing = 13;     

// Electronics Box
box_w = 36;
box_l = 36;
box_h = 25;
box_wall = 3;
box_radius = 4;

// Screw Settings
screw_clearance_diam = 3.8; // Hole for screw shaft
pilot_hole_diam = 2.8;      // Hole for screw threads (in top part)

// --- NEW COUNTERBORE SETTINGS ---
screw_head_diam = 7.0;      // Requested 7mm
// The shelf is where the screw head touches. 
// Z=0 is the meeting line. Z=-5 means there is 5mm of plastic holding the clamp.
screw_head_shelf_z = -10.0;  

// Calculated screw position
screw_offset = (handle_diam/2) + extra_spacing; 

// Wing Thickness
wing_thick = 8;         

$fn = 60;

// --- Logic ---

if (part_to_print == 0) {
    // Assembly View
    translate([0,0,5]) Top_Housing();
    translate([0,0,-5]) Bottom_Clamp();
    
    // Hardware Preview
    %color("Red") rotate([0,90,0]) cylinder(d=handle_diam, h=100, center=true);
    
    // Show Screw Head Landing Spot (Green Disk)
    // This represents the flat surface the screw pushes against
    color("Lime") translate([0, screw_offset, -5 + screw_head_shelf_z]) cylinder(d=screw_head_diam, h=0.2);
    color("Lime") translate([0, -screw_offset, -5 + screw_head_shelf_z]) cylinder(d=screw_head_diam, h=0.2);
}
else if (part_to_print == 1) {
    rotate([180,0,0]) Top_Housing();
}
else if (part_to_print == 2) {
    Bottom_Clamp();
}

// --- Modules ---

module Top_Housing() {
    color("CornflowerBlue")
    difference() {
        hull() {
            translate([-box_w/2, -box_l/2, 0])
                Rounded_Block(box_w, box_l, box_h + box_wall, box_radius);
            
            translate([0, screw_offset, 0])
                cylinder(d=clamp_width, h=wing_thick, center=false); 
            translate([0, -screw_offset, 0])
                cylinder(d=clamp_width, h=wing_thick, center=false); 
            
             translate([-clamp_width/2, -handle_diam/2, 0])
                    cube([clamp_width, handle_diam, wing_thick]); 
        }

        // Hollow
        translate([-box_w/2 + box_wall, -box_l/2 + box_wall, -0.1])
            Rounded_Block(box_w - 2*box_wall, box_l - 2*box_wall, box_h, box_radius-1);
            
        // Handle Cutout
        translate([0,0,0])
            rotate([0,90,0]) 
            cylinder(d=handle_diam, h=clamp_width+50, center=true);
            
        // Pilot Holes (Blind)
        hole_depth = wing_thick + 5; 
        translate([0, screw_offset, -0.1])
            cylinder(d=pilot_hole_diam, h=hole_depth);
        translate([0, -screw_offset, -0.1])
            cylinder(d=pilot_hole_diam, h=hole_depth);
            
        // Encoder Hole
        translate([0,0, box_h + box_wall/2])
            cylinder(d=7.5, h=20, center=true);
    }
}

module Bottom_Clamp() {
    color("Orange")
    difference() {
        union() {
            hull() {
                translate([0, screw_offset, -wing_thick])
                    cylinder(d=clamp_width, h=wing_thick, center=false); 
                translate([0, -screw_offset, -wing_thick])
                    cylinder(d=clamp_width, h=wing_thick, center=false);
                
                translate([-clamp_width/2, -handle_diam/2 - 2, -20])
                    cube([clamp_width, handle_diam + 4, 20]);
            }
        }
            
        // Handle Cutout
        translate([0,0,0])
            rotate([0,90,0]) 
            cylinder(d=handle_diam, h=clamp_width+50, center=true);
            
        // 1. Through Holes
        translate([0, screw_offset, -50]) cylinder(d=screw_clearance_diam, h=100);
        translate([0, -screw_offset, -50]) cylinder(d=screw_clearance_diam, h=100);
        
        // 2. COUNTERBORE (The Flat Shelf)
        // We cut from way below (-50) UP TO the shelf level.
        // This guarantees a flat spot even if the hull is curved/angled.
        
        // Hole 1
        translate([0, screw_offset, -50])
            cylinder(d=screw_head_diam, h=50 + screw_head_shelf_z);
            
        // Hole 2
        translate([0, -screw_offset, -50])
            cylinder(d=screw_head_diam, h=50 + screw_head_shelf_z);
    }
}

module Rounded_Block(w, l, h, r) {
    hull() {
        translate([r, r, 0]) cylinder(r=r, h=h);
        translate([w-r, r, 0]) cylinder(r=r, h=h);
        translate([w-r, l-r, 0]) cylinder(r=r, h=h);
        translate([r, l-r, 0]) cylinder(r=r, h=h);
    }
}