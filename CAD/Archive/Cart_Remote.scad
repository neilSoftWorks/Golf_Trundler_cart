// --- 1. PARAMETERS ---

// Which part to see?
part_to_show = "both"; // [both, base, lid]
show_labels  = false;  

// --- REMOTE DIMENSIONS (Ultra-Slim Profile) ---
width  = 48;    // Enough for 30mm battery + wire routing
length = 115;   // TP4056(28) + Battery(50) + Switch(35)
base_depth = 9; // Fits the 6mm thick battery + tape
lid_depth  = 7; // Fits the switch housing above the PCB

// --- WALL THICKNESS ---
wall = 1.6;          
lid_thickness = 1.2; 
gap  = 0.2;          
lip_wall = 1.2;      

// --- NAV SWITCH (D-PAD) ---
nav_dia = 36.5;         
nav_y_center = 92.0;    // Positioned at the top for thumb access

// --- NAV SWITCH MOUNTING (Veroboard - In Lid) ---
nav_mount_spacing_x = 28.0; 
nav_mount_spacing_y = 28.0; 

nav_peg_dia = 2.0;       // Diameter of the pin for the PCB hole
nav_peg_height = 4.0;    // Height of the pin 
nav_foot_dia = 5.0;      // Base pillar diameter
nav_foot_height = 2.0;   // Drops the board 2mm from the ceiling

// --- TP4056 USB-C PORT (Bottom Edge) ---
usbc_w = 9.3;
usbc_h = 3.5;
usbc_gap_from_floor = 1.0; 

// Radii
corner_rad = 6;      
edge_rad   = 2.5;    

// Screw Dimensions (M3)
screw_dia = 3.2 + 0.6;       
screw_pilot = 2.7;     
screw_head_dia = 4.8 + 0.6; 
screw_head_height = 3; 
screw_pos_offset = 5;  

// View Control
explode_view = 25; 
cross_section = false; 

$fn = 60; 
overlap = 0.1; 
fuse_fix = 0.2; 

// --- 2. ASSEMBLY ---
difference() {
    union() {
        if (part_to_show == "base" || part_to_show == "both") {
            color("CornflowerBlue") base();
        }

        if (part_to_show == "lid" || part_to_show == "both") {
            z_pos = (part_to_show == "both") ? base_depth + explode_view : 0;
            rot_vec = (part_to_show == "lid") ? [180,0,0] : [0,0,0];
            
            translate([0, 0, z_pos]) 
                rotate(rot_vec)
                color("Orange") lid();
        }
    }
    
    if (cross_section) {
        translate([-5, -50, -50]) cube([100, 200, 200]);
    }
}

// --- 3. MODULES ---

module base() {
    difference() {
        union() {
            difference() {
                intersection() {
                    soft_box(width, length, base_depth + edge_rad, corner_rad, edge_rad);
                    cube([width, length, base_depth]);
                }
                translate([wall, wall, wall])
                    rounded_prism(width - 2*wall, length - 2*wall, base_depth, corner_rad - wall);
            }
            
            // FIX: Clip base pillars against the outer rounded shell
            intersection() {
                soft_box(width, length, base_depth + edge_rad, corner_rad, edge_rad);
                pillar_positions(base_depth - 2.6);
            }
        }
        
        screw_locations("clearance");
        
        // USB-C Cutout for TP4056 (Centered on the narrow bottom edge)
        z_pos_usbc = wall + usbc_gap_from_floor + (usbc_h / 2);
        translate([width/2, 0, z_pos_usbc])
            rotate([90, 0, 0]) usbc_shape_extruded();
    }
}

module lid() {
    difference() {
        union() {
            // A. Solid Lid Shell
            difference() {
                union() {
                     // Main Body
                    intersection() {
                        translate([0, 0, -edge_rad])
                            soft_box(width, length, lid_depth + edge_rad, corner_rad, edge_rad);
                        cube([width, length, lid_depth]);
                    }
                    
                    // The Lip
                    translate([wall + gap - fuse_fix, wall + gap - fuse_fix, -2]) 
                        rounded_prism(
                            width - 2*(wall + gap - fuse_fix), 
                            length - 2*(wall + gap - fuse_fix), 
                            2 + overlap, 
                            corner_rad - (wall - fuse_fix)
                        );
                }

                // Hollow Inside
                union() {
                    translate([wall + gap, wall + gap, -overlap])
                        rounded_prism(width - 2*(wall + gap), length - 2*(wall + gap), lid_depth - lid_thickness + overlap, corner_rad - (wall+gap));
                    
                    translate([wall + gap + lip_wall, wall + gap + lip_wall, -2.1])
                        rounded_prism(width - 2*(wall + gap + lip_wall), length - 2*(wall + gap + lip_wall), 2.1 + overlap, corner_rad - (wall + gap + lip_wall));
                }
            }
            
            // FIX: Clip lid pillars against the outer rounded shell
            intersection() {
                translate([0, 0, -edge_rad])
                    soft_box(width, length, lid_depth + edge_rad, corner_rad, edge_rad);
                pillar_positions(lid_depth);
            }
            
            nav_mount_pegs(); // Veroboard pegs pointing down from the ceiling
        }

        // Nav Switch Cutout
        translate([width/2, nav_y_center, -10])
            cylinder(h=30, d=nav_dia);

        screw_locations("pilot");
    }
}

// --- HELPER MODULES ---

module nav_mount_pegs() {
    // Start at the ceiling of the lid
    start_z = lid_depth - lid_thickness - overlap;
    
    // Create a 2x2 grid of mounting pegs centered around the switch cutout
    for (x = [-nav_mount_spacing_x/2, nav_mount_spacing_x/2]) {
        for (y = [-nav_mount_spacing_y/2, nav_mount_spacing_y/2]) {
            
            translate([width/2 + x, nav_y_center + y, start_z])
                mirror([0,0,1]) // Point them downwards
                union() {
                    cylinder(h = nav_foot_height, d = nav_foot_dia);
                    translate([0, 0, nav_foot_height - 0.01])
                        cylinder(h = nav_peg_height, d = nav_peg_dia);
                }
        }
    }
}

module usbc_shape_extruded() {
    linear_extrude(20, center=true) 
        hull() {
            translate([-(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
            translate([(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
        }
}

module pillar_positions(h) {
    positions = [
        [screw_pos_offset, screw_pos_offset], 
        [width-screw_pos_offset, screw_pos_offset], 
        [screw_pos_offset, length-screw_pos_offset], 
        [width-screw_pos_offset, length-screw_pos_offset]
    ];
    for(pos = positions) {
        translate([pos[0], pos[1], 0])
             cylinder(h=h, r=screw_pos_offset - wall + overlap); 
    }
}

module soft_box(w, l, h, c_rad, e_rad) {
    translate([e_rad, e_rad, e_rad])
        minkowski() {
            rounded_prism(w - 2*e_rad, l - 2*e_rad, h - 2*e_rad, c_rad - e_rad);
            sphere(r=e_rad);
        }
}

module rounded_prism(w, l, h, r) {
    hull() {
        translate([r, r, 0]) cylinder(h=h, r=r);
        translate([w-r, r, 0]) cylinder(h=h, r=r);
        translate([r, l-r, 0]) cylinder(h=h, r=r);
        translate([w-r, l-r, 0]) cylinder(h=h, r=r);
    }
}

module screw_locations(type) {
    positions = [
        [screw_pos_offset, screw_pos_offset], 
        [width-screw_pos_offset, screw_pos_offset], 
        [screw_pos_offset, length-screw_pos_offset], 
        [width-screw_pos_offset, length-screw_pos_offset]
    ];

    for (pos = positions) {
        translate([pos[0], pos[1], 0]) {
            if (type == "clearance") {
                translate([0,0,-1]) cylinder(h = base_depth + 2, d = screw_dia);
                translate([0,0,-0.1]) cylinder(h = screw_head_height, d = screw_head_dia); 
            }
            if (type == "pilot") {
                translate([0,0,-2.1]) cylinder(h = lid_depth - 1, d = screw_pilot);
            }
        }
    }
}