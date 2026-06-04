// --- PART SELECTOR ---
// 0 = Full Assembly View & Hardware Layout (Transparent)
// 1 = Base Box (Print flat)
// 2 = Lid (Print flat on top face)
part_to_show = 0; 

// --- 1. ENCLOSURE PARAMETERS ---
width  = 40;     
length = 80;     
base_depth = 12; 
lid_depth  = 5;  

wall = 1.6;          
lid_thickness = 1.2; 
gap  = 0.2;          
lip_wall = 1.2;      

// --- 2. HARDWARE CUTOUTS ---
// 5-Way Switch
switch_hole_dia = 9.5;    
switch_y_pos = length - 13; 

// TP4056 USB-C Charger (FLIPPED UPSIDE DOWN)
usbc_w = 9.5;
usbc_h = 4.5; 
usbc_z_center = wall + 2.0; 

// --- 3. HARDWARE PREVIEW DIMENSIONS ---
batt_w = 30;
batt_l = 50;
batt_h = 6;

tp_w = 15;
tp_l = 28;
tp_h = 4.1; 

esp_w = 18;
esp_l = 22.5;
esp_h = 5.0; 

// Radii & Screws
corner_rad = 4;      
edge_rad   = 2;      
box_screw_dia = 3.0;        
box_screw_pilot = 2.6;      
box_screw_head_dia = 5.6; 
box_screw_head_height = 3; 
box_pos_offset = 5;  

$fn = 60; 
overlap = 0.1; 

// --- ASSEMBLY LOGIC ---
if (part_to_show == 0) {
    color("CornflowerBlue", 0.8) base();
    
    translate([0, 0, base_depth + 15]) 
        rotate([180,0,0]) color("Orange", 0.8) lid();
        
    // --- HARDWARE PREVIEW ---
    // 1. TP4056 Charger (Upside down on floor)
    %color("DarkBlue") translate([width/2, wall + tp_l/2, wall + (tp_h/2)]) 
        cube([tp_w, tp_l, tp_h], center=true);
        
    // 2. ESP32-C3 SuperMini 
    %color("Black") translate([width/2, wall + tp_l + (esp_l/2) + 2, wall + (esp_h/2)]) 
        cube([esp_w, esp_l, esp_h], center=true);
        
    // 3. Battery (Shifted down to rest just above the bottom screw pillars)
    // The bottom pillars end at ~8.5mm. Starting the battery at 10mm.
    batt_y_center = 10 + (batt_l/2); 
    %color("Silver") translate([width/2, batt_y_center, wall + 5.0 + (batt_h/2)]) 
        cube([batt_w, batt_l, batt_h], center=true);
        
    // 4. 5-Way Switch 
    %color("DarkRed") translate([width/2, switch_y_pos, base_depth - 1]) 
        cube([10, 10, 4], center=true);
}
else if (part_to_show == 1) {
    base();
}
else if (part_to_show == 2) {
    rotate([180,0,0]) lid();
}

// --- MODULES ---

module base() {
    difference() {
        union() {
            // Main Shell
            difference() {
                intersection() {
                    soft_box(width, length, base_depth + edge_rad, corner_rad, edge_rad);
                    cube([width, length, base_depth]);
                }
                translate([wall, wall, wall])
                    rounded_prism(width - 2*wall, length - 2*wall, base_depth, corner_rad - wall);
            }
            pillar_positions(base_depth - 2);
            
            // TP4056 Backstop
            translate([width/2, wall + tp_l, wall + 1.5])
                cube([tp_w + 2, 3, 3], center=true);
                
            // TP4056 Alignment Rails
            translate([width/2 - (tp_w/2) - 1, wall + 10, wall + 1.5])
                cube([2, 20, 3], center=true);
            translate([width/2 + (tp_w/2) + 1, wall + 10, wall + 1.5])
                cube([2, 20, 3], center=true);
        }
        
        box_screw_locations("clearance");
        
        // USB-C Cutout 
        translate([width/2, 0, usbc_z_center]) 
            rotate([90, 0, 0]) linear_extrude(20, center=true) hull() {
                translate([-(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
                translate([(usbc_w - usbc_h)/2, 0, 0]) circle(d=usbc_h, $fn=30);
            }
            
        // Sealed Light-Bleed Grid in the Base
        led_grid();
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
            translate([0,0,-2]) pillar_positions(lid_depth + 2);
        }

        // 5-Way Switch Cutout
        translate([width/2, switch_y_pos, -5]) 
            cylinder(d=switch_hole_dia, h=20);
            
        box_screw_locations("pilot");
    }
}

// --- HELPER MODULES ---

module led_grid() {
    grid_y_center = 10; 
    grid_spacing = 3.0; 
    hole_dia = 2.0;
    
    cut_height = wall - 0.4; 
    
    for (x = [-1.5, 0, 1.5]) {        
        for (y = [-1.5, -0.5, 0.5, 1.5]) {  
            translate([
                (width/2) + (x * grid_spacing), 
                grid_y_center + (y * grid_spacing), 
                -5 
            ])
            cylinder(d=hole_dia, h=5 + cut_height, $fn=16);
        }
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
            translate([0,0,-2.1]) cylinder(h = lid_depth + 1, d = box_screw_pilot);
        }
    }
}