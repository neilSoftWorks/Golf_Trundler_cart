// --- PART SELECTOR ---
// 0 = Full Assembly View (Exploded)
// 1 = Base Box (Print flat)
// 2 = Lid (Print flat on top face)
// 3 = Bottom Clamp (Print flat on bottom)
// 4 = Top Clamp Adapter (Print flat on TOP face)
part_to_show = 0; 

// --- 1. ENCLOSURE PARAMETERS ---
width  = 42;
length = 80; 
base_depth = 12; 
lid_depth  = 5.5;

wall = 1.6;          
lid_thickness = 1.2; 
gap  = 0.2;          
lip_wall = 1.2;      

// --- 2. MOUNTING CLAMP PARAMETERS ---
handle_diam = 32;       
clamp_width = 30;       
extra_spacing = 13;     
wing_thick = 8;  

clamp_y_center = length / 2;    
clamp_height = (handle_diam / 2) + 6; 

screw_clearance_diam = 3.8; 
pilot_hole_diam = 2.8;      
screw_head_diam = 7.0;      
screw_offset = (handle_diam/2) + extra_spacing; 

// Box-to-Clamp Internal Mounting
box_mount_spacing = 24; 
box_mount_head_dia = 6.5; 
cable_hole_dia = 10.0;  

// --- 3. HARDWARE CUTOUTS ---
tft_pcb_w = 30.0;    
tft_pcb_l = 43.0;    
tft_screen_w = 27.0; 
tft_screen_l = 29.0; 
tft_margin_y = 4.0;  
tft_tolerance = 0.6; 
tft_screen_offset_y = 0.0; 

encoder_hole_dia = 7.5;     
encoder_y_center = 66.0;   

// --- 4. PORTS & BACKSTOPS ---
usbc_w = 9.3;
usbc_h = 3.5;
usbc_gap_from_floor = 1.0; 
usbc_y_pos = 19.0;  // Shifted away from the central mounting boss

backstop_dist_from_wall = 24.1; 
backstop_length = 10.0; 
backstop_thick  = 1.6;  
backstop_height = 3.0;  

// Radii & Box Screws
corner_rad = 4;      
edge_rad   = 2;      
box_screw_dia = 3.8;        
box_screw_pilot = 2.7;      
box_screw_head_dia = 5.4; 
box_screw_head_height = 3; 
box_pos_offset = 5;  

$fn = 60; 
overlap = 0.1; 

// --- ASSEMBLY LOGIC ---
if (part_to_show == 0) {
    translate([0, 0, clamp_height]) color("CornflowerBlue") base();
    
    translate([0, 0, clamp_height + base_depth + 15]) 
        rotate([180,0,0]) color("Orange") lid();
        
    color("DodgerBlue") top_clamp();
        
    translate([0, 0, -10]) color("DarkOrange") bottom_clamp();
        
    %color("Red") translate([width/2, clamp_y_center, 0]) 
        rotate([90,0,0]) cylinder(d=handle_diam, h=length + 20, center=true);
        
    %color("LimeGreen") translate([width/2, clamp_y_center, 0]) 
        cylinder(d=cable_hole_dia-1, h=clamp_height + base_depth + 5);
}
else if (part_to_show == 1) {
    base();
}
else if (part_to_show == 2) {
    rotate([180,0,0]) lid();
}
else if (part_to_show == 3) {
    bottom_clamp();
}
else if (part_to_show == 4) {
    rotate([180,0,0]) translate([0, 0, -clamp_height]) top_clamp();
}

// --- MODULES ---

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
            
            translate([width/2, clamp_y_center, wall + 1.5])
                cube([box_mount_spacing + 14, 14, 3], center=true);
                
            usb_backstop_right();
        }
        box_screw_locations("clearance");
        usb_cutouts(); 
        box_mount_holes();
        
        translate([width/2, clamp_y_center, -5])
            cylinder(d=cable_hole_dia, h=base_depth + 10);
    }
}

module top_clamp() {
    translate([width/2, clamp_y_center, 0]) {
        difference() {
            hull() {
                translate([screw_offset, 0, 0]) cylinder(d=clamp_width, h=wing_thick); 
                translate([-screw_offset, 0, 0]) cylinder(d=clamp_width, h=wing_thick); 
                translate([0, 0, clamp_height - 1]) cube([clamp_width, clamp_width, 2], center=true);
            }
            
            rotate([90,0,0]) cylinder(d=handle_diam, h=clamp_width+50, center=true);
            
            translate([screw_offset, 0, -1]) cylinder(d=pilot_hole_diam, h=wing_thick);
            translate([-screw_offset, 0, -1]) cylinder(d=pilot_hole_diam, h=wing_thick);
            
            translate([box_mount_spacing/2, 0, clamp_height - 10]) cylinder(d=pilot_hole_diam, h=15);
            translate([-box_mount_spacing/2, 0, clamp_height - 10]) cylinder(d=pilot_hole_diam, h=15);
            
            translate([0, 0, -50]) cylinder(d=cable_hole_dia, h=100);
        }
    }
}

module bottom_clamp() {
    translate([width/2, clamp_y_center, 0]) {
        difference() {
            hull() {
                translate([screw_offset, 0, -wing_thick]) cylinder(d=clamp_width, h=wing_thick); 
                translate([-screw_offset, 0, -wing_thick]) cylinder(d=clamp_width, h=wing_thick);
                translate([0, 0, -clamp_height + 1]) cube([clamp_width, clamp_width, 2], center=true);
            }
                
            rotate([90,0,0]) cylinder(d=handle_diam, h=clamp_width+50, center=true);
            
            translate([screw_offset, 0, -50]) cylinder(d=screw_clearance_diam, h=100);
            translate([-screw_offset, 0, -50]) cylinder(d=screw_clearance_diam, h=100);
            
            cb_depth = 5;
            translate([screw_offset, 0, -clamp_height - 1]) cylinder(d=screw_head_diam, h=cb_depth + 1);
            translate([-screw_offset, 0, -clamp_height - 1]) cylinder(d=screw_head_diam, h=cb_depth + 1);
        }
    }
}

module lid() {
    difference() {
        union() {
            difference() {
                union() {
                    intersection() {
                        translate([0, 0, -edge_rad]) soft_box(width, length, lid_depth + edge_rad, corner_rad, edge_rad);
                        cube([width, length, lid_depth]);
                    }
                    translate([wall + gap - 0.2, wall + gap - 0.2, -2]) 
                        rounded_prism(width - 2*(wall + gap - 0.2), length - 2*(wall + gap - 0.2), 2 + overlap, corner_rad - (wall - 0.2));
                }
                union() {
                    translate([wall + gap, wall + gap, -overlap])
                        rounded_prism(width - 2*(wall + gap), length - 2*(wall + gap), lid_depth - lid_thickness + overlap, corner_rad - (wall+gap));
                    translate([wall + gap + lip_wall, wall + gap + lip_wall, -2.1])
                        rounded_prism(width - 2*(wall + gap + lip_wall), length - 2*(wall + gap + lip_wall), 2.1 + overlap, corner_rad - (wall + gap + lip_wall));
                }
            }
            pillar_positions(lid_depth);
            tft_cradle(); 
        }

        screen_cut_x = (width - tft_screen_w) / 2;
        screen_cut_y = (wall + gap + lip_wall) + tft_margin_y + ((tft_pcb_l - tft_screen_l)/2) + tft_screen_offset_y;
        
        translate([screen_cut_x, screen_cut_y, -5]) cube([tft_screen_w, tft_screen_l, 20]);  
        translate([width/2, encoder_y_center, -5]) cylinder(h=20, d=encoder_hole_dia);
        box_screw_locations("pilot");
    }
}

// --- HELPER MODULES ---

module usb_backstop_right() {
    x_pos = width - wall - backstop_dist_from_wall - (backstop_thick / 2);
    y_pos = usbc_y_pos;
    z_pos = wall + (backstop_height / 2);
    translate([x_pos, y_pos, z_pos]) cube([backstop_thick, backstop_length, backstop_height], center=true);
}

module box_mount_holes() {
    boss_z = wall + 3; 
    cone_h = 2.0;      
    
    translate([width/2 + box_mount_spacing/2, clamp_y_center, 0]) {
        translate([0,0,-1]) cylinder(d=screw_clearance_diam, h=base_depth + 2);
        translate([0,0, boss_z - cone_h]) cylinder(d1=screw_clearance_diam, d2=box_mount_head_dia, h=cone_h + 0.1);
        translate([0,0, boss_z]) cylinder(d=box_mount_head_dia, h=base_depth);
    }
    translate([width/2 - box_mount_spacing/2, clamp_y_center, 0]) {
        translate([0,0,-1]) cylinder(d=screw_clearance_diam, h=base_depth + 2);
        translate([0,0, boss_z - cone_h]) cylinder(d1=screw_clearance_diam, d2=box_mount_head_dia, h=cone_h + 0.1);
        translate([0,0, boss_z]) cylinder(d=box_mount_head_dia, h=base_depth);
    }
}

module tft_cradle() {
    pcb_w = tft_pcb_w + tft_tolerance;
    pcb_l = tft_pcb_l + tft_tolerance;
    wall_thick = 1.6;
    cradle_h = 2.5; 
    
    cradle_x = (width - pcb_w) / 2;
    cradle_y = (wall + gap + lip_wall) + tft_margin_y;
    start_z = lid_depth - lid_thickness;
    
    translate([cradle_x, cradle_y, start_z]) mirror([0,0,1]) difference() {
        translate([-wall_thick, -wall_thick, 0]) cube([pcb_w + 2*wall_thick, pcb_l + 2*wall_thick, cradle_h]);
        translate([0, 0, -1]) cube([pcb_w, pcb_l, cradle_h + 2]);
        translate([pcb_w/4, -wall_thick - 1, -1]) cube([pcb_w/2, wall_thick + 2, cradle_h + 2]);
    }
}

module usb_cutouts() {
    z_pos_usbc  = wall + usbc_gap_from_floor + (usbc_h / 2);
    translate([width, usbc_y_pos, z_pos_usbc]) rotate([90, 0, 90]) linear_extrude(20, center=true) hull() {
        translate([-(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
        translate([(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
    }
}

module pillar_positions(h) {
    positions = [
        [box_pos_offset, box_pos_offset], 
        [width-box_pos_offset, box_pos_offset], 
        [box_pos_offset, length-box_pos_offset], 
        [width-box_pos_offset, length-box_pos_offset]
    ];
    for(pos = positions) translate([pos[0], pos[1], 0]) cylinder(h=h, r=box_pos_offset - wall + overlap); 
}

module soft_box(w, l, h, c_rad, e_rad) {
    translate([e_rad, e_rad, e_rad]) minkowski() {
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

module box_screw_locations(type) {
    positions = [
        [box_pos_offset, box_pos_offset], 
        [width-box_pos_offset, box_pos_offset], 
        [box_pos_offset, length-box_pos_offset], 
        [width-box_pos_offset, length-box_pos_offset]
    ];
    for (pos = positions) translate([pos[0], pos[1], 0]) {
        if (type == "clearance") {
            translate([0,0,-1]) cylinder(h = base_depth + 2, d = box_screw_dia);
            translate([0,0,-0.1]) cylinder(h = box_screw_head_height, d = box_screw_head_dia); 
        }
        if (type == "pilot") {
            translate([0,0,-2.1]) cylinder(h = lid_depth - 1, d = box_screw_pilot);
        }
    }
}