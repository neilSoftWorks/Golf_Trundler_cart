// --- 1. PARAMETERS ---

// Which part to see?
part_to_show = "lid"; // [both, base, lid]
show_labels  = false;  

width  = 54;
length = 120; 
base_depth = 10;
lid_depth  = 5.5;

// --- WALL THICKNESS ---
wall = 1.6;          
lid_thickness = 1.2; 
gap  = 0.2;          
lip_wall = 1.2;      

// --- TFT CUTOUT ---
tft_width  = 33;
tft_height = 43;
tft_margin = 13; 

// --- TFT PEGS ---
peg_dia = 2;
peg_height = 6;
peg_foot_height = 2; 
peg_foot_dia    = 5; 

// --- NAV SWITCH (D-PAD) ---
nav_dia = 36.5;         
nav_y_center = 91.0;    

// --- NAV SWITCH MOUNTING (Veroboard - In Base) ---
nav_mount_spacing_x = 28.0; 
nav_mount_spacing_y = 28.0; 

nav_peg_dia = 2.0;       // Diameter of the pin going through the PCB
nav_peg_height = 3.0;    // Height of the pin 
nav_foot_dia = 5.0;      // Base pillar diameter
nav_foot_height = 5.0;   // ADJUST THIS to set how high the PCB sits off the floor!

// --- PORT CUTOUTS (Right Long Edge) ---
// 1. USB-C Port (Side Wall)
usbc_w = 9.3;
usbc_h = 3.5;
usbc_gap_from_floor = 1.0; 
usbc_y_pos = 19.0; 

// 2. Rectangular Floor Port 
floor_hole_length = 13.0; 
floor_hole_width  = 6.0;  
// Positioned automatically at length/2 (Centered)

// --- PORT BACKSTOPS ---
backstop_right_dist_from_wall = 24.1; 
backstop_right_length = 10; 
backstop_right_thick  = 1.6;  
backstop_right_height = 3;  

// Radii
corner_rad = 4;      
edge_rad   = 2;    

// Screw Dimensions (M3)
screw_dia = 3.2 + 0.6;       
screw_pilot = 2.7;     
screw_head_dia = 4.8 + 0.6; 
screw_head_height = 3; 
screw_pos_offset = 5;  

// View Control
explode_view = 25; 
cross_section = false; 

$fn = 50; 
overlap = 0.1; 
fuse_fix = 0.2; 

// --- 2. ASSEMBLY ---
difference() {
    union() {
        if (part_to_show == "base" || part_to_show == "both") {
            color("CornflowerBlue") base();
            if (show_labels) orientation_labels();
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
            
            pillar_positions(base_depth - 2.6);
            usb_backstop_right(); 
            
            // Veroboard pillars moved here, pointing UP from the base
            nav_mount_pegs(); 
        }
        
        screw_locations("clearance");
        usb_cutouts(); 
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
            
            pillar_positions(lid_depth);
            tft_pegs();
        }

        // TFT Cutout
        translate([
            (width - tft_width) / 2,                
            (wall + gap + lip_wall) + tft_margin,   
            -5                                      
        ])
        cube([tft_width, tft_height, 20]);  

        // Nav Switch Cutout
        nav_switch();

        screw_locations("pilot");
    }
}

// --- HELPER MODULES ---

module nav_switch() {
    // A single circular hole for the entire D-pad cap assembly
    translate([width/2, nav_y_center, -10])
        cylinder(h=30, d=nav_dia);
}

module nav_mount_pegs() {
    // Start at the floor of the base
    start_z = wall; 
    
    // Create a 2x2 grid of mounting pegs centered around the switch cutout
    for (x = [-nav_mount_spacing_x/2, nav_mount_spacing_x/2]) {
        for (y = [-nav_mount_spacing_y/2, nav_mount_spacing_y/2]) {
            
            translate([width/2 + x, nav_y_center + y, start_z])
                union() {
                    // The wide base pillar the PCB rests on
                    cylinder(h = nav_foot_height, d = nav_foot_dia);
                    
                    // The narrow pin that goes through the PCB hole
                    translate([0, 0, nav_foot_height - 0.01])
                        cylinder(h = nav_peg_height, d = nav_peg_dia);
                }
        }
    }
}

module usb_backstop_right() {
    x_pos = (width - wall) - backstop_right_dist_from_wall;
    y_pos = usbc_y_pos;
    z_pos = wall + (backstop_right_height / 2);
    
    translate([x_pos, y_pos, z_pos])
        cube([backstop_right_thick, backstop_right_length, backstop_right_height], center=true);
}

module usb_cutouts() {
    // 1. USB-C Pill Cutout (Side Wall)
    z_pos_usbc  = wall + usbc_gap_from_floor + (usbc_h / 2);
    translate([width, usbc_y_pos, z_pos_usbc])
        rotate([90, 0, 90]) usbc_shape_extruded();

    // 2. Rectangular Floor Cutout (Centered along the right edge)
    translate([width - wall - (floor_hole_width / 2) - 1, length / 2, 0])
        cube([floor_hole_width, floor_hole_length, 20], center=true);
}

module usbc_shape_extruded() {
    linear_extrude(20, center=true) 
        hull() {
            translate([-(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
            translate([(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
        }
}

module tft_pegs() {
    cutout_x = (width - tft_width) / 2;
    cutout_y = (wall + gap + lip_wall) + tft_margin;
    start_z = lid_depth - lid_thickness - overlap;
    
    peg_positions = [
        [2.5, tft_height + 10], 
        [tft_width - 2.5, tft_height + 10],
        [2.5, -5.3],
        [tft_width - 2.5, -5.3]
    ];
    
    for (pos = peg_positions) {
        translate([cutout_x + pos[0], cutout_y + pos[1], start_z])
            mirror([0,0,1])
            union() {
                cylinder(h = peg_height, d = peg_dia);
                cylinder(h = peg_foot_height, d = peg_foot_dia);
            }
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

module orientation_labels() {
    color("Red") {
        translate([width/2, length - 8, wall])
            linear_extrude(0.6)
            text("TOP", size=5, halign="center", valign="center", font="Liberation Sans:style=Bold");

        translate([width/2, 8, wall])
            linear_extrude(0.6)
            text("BTM", size=5, halign="center", valign="center", font="Liberation Sans:style=Bold");

        translate([8, length/2, wall])
            linear_extrude(0.6)
            text("L", size=5, halign="center", valign="center", font="Liberation Sans:style=Bold");

        translate([width - 8, length/2, wall])
            linear_extrude(0.6)
            text("R", size=5, halign="center", valign="center", font="Liberation Sans:style=Bold");
    }
}