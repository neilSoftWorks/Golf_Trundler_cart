// --- PARAMETERS ---

// 1. Dimensions
inner_w = 35.0; // Width to fit GPS
outer_l = 32.0; // Total length from Top to Bottom edge

// 2. Heights
tall_z  = 14.0; // Top wall height
short_z = 10.0;  // Left and Right wall height

// 3. Wall Settings
wall = 1.6;     // Wall thickness
floor = 3.6;    // INCREASED: 3.6mm to fit a 2mm magnet + 1.6mm solid floor
rad = 2.0;      // Outer corner rounding radius (XY plane)

// 4. 3D Fillet Radius
edge_rad = 0.6; // Rounds ALL sharp 90-deg edges

// 5. The Sweeps & Curves
trans_rad = 4.0;       // Radius of the curved join
top_corner_rad = 5.0;  // Radius to round off the sharp points

// 6. Mounting Holes (Countersunk)
mount_hole_dia = 3.2;    // 3mm hole + 0.2mm clearance
mount_head_dia = 6.2;    // Diameter of the countersunk screw head 
mount_head_depth = 1.5;  // Depth of the countersink cone 
mount_y_from_top = 4.5;  // Distance from the top wall
mount_x_spacing = 14.0;  // Distance between the two holes

// 7. Magnet Recess (Underneath)
mag_dia = 20.4;    // 20mm magnet + 0.4mm for clearance/glue
mag_depth = 2.2;   // 2mm depth + 0.2mm clearance so it sits flush
mag_y_pos = 14.0;  // Placed exactly between the open edge and the screw holes

$fn = 40; 

// --- CALCULATIONS ---
outer_w = inner_w + (wall * 2);

// --- SKELETON CALCULATIONS ---
sk_outer_w = outer_w - (edge_rad * 2);
sk_inner_w = inner_w + (edge_rad * 2);
sk_outer_l = outer_l - edge_rad;
sk_floor_z = edge_rad;
sk_floor_top = floor - edge_rad;
sk_tall_z = tall_z - edge_rad;
sk_short_z = short_z - edge_rad;
sk_wall = wall - (edge_rad * 2); 
sk_rad = rad - edge_rad;
sk_irad = max(0.1, sk_rad - sk_wall);

// --- SKELETON MODULES ---
module sk_side_cutter() {
    sk_trans_rad = trans_rad + edge_rad;
    cutter_y_end = sk_outer_l - sk_wall - sk_trans_rad;
    hull() {
        translate([0, cutter_y_end, sk_short_z + sk_trans_rad]) rotate([0, 90, 0]) cylinder(r=sk_trans_rad, h=sk_outer_w+10, center=true);
        translate([0, cutter_y_end, sk_tall_z + 10]) rotate([0, 90, 0]) cylinder(r=sk_trans_rad, h=sk_outer_w+10, center=true);
        translate([0, -10, sk_short_z + sk_trans_rad]) rotate([0, 90, 0]) cylinder(r=sk_trans_rad, h=sk_outer_w+10, center=true);
        translate([0, -10, sk_tall_z + 10]) rotate([0, 90, 0]) cylinder(r=sk_trans_rad, h=sk_outer_w+10, center=true);
    }
}

module sk_top_corner_rounder() {
    sk_tcr = top_corner_rad - edge_rad;
    rotate([90, 0, 0])
    linear_extrude(sk_outer_l * 3, center=true)
    hull() {
        translate([-sk_outer_w/2, 0]) square([sk_outer_w, sk_tall_z - sk_tcr]);
        translate([-sk_outer_w/2 + sk_tcr, sk_tall_z - sk_tcr]) circle(r=sk_tcr);
        translate([ sk_outer_w/2 - sk_tcr, sk_tall_z - sk_tcr]) circle(r=sk_tcr);
    }
}

module skeleton() {
    intersection() {
        difference() {
            // A. Solid Block
            hull() {
                translate([-sk_outer_w/2 + sk_rad, sk_outer_l - sk_rad, sk_floor_z])
                    cylinder(r=sk_rad, h=sk_tall_z - sk_floor_z);
                translate([ sk_outer_w/2 - sk_rad, sk_outer_l - sk_rad, sk_floor_z])
                    cylinder(r=sk_rad, h=sk_tall_z - sk_floor_z);
                translate([-sk_outer_w/2, edge_rad, sk_floor_z]) 
                    cube([sk_outer_w, 0.1, sk_tall_z - sk_floor_z]);
            }

            // B. Inner Void
            hull() {
                translate([-sk_inner_w/2 + sk_irad, sk_outer_l - sk_wall - sk_irad, sk_floor_top])
                    cylinder(r=sk_irad, h=sk_tall_z);
                translate([ sk_inner_w/2 - sk_irad, sk_outer_l - sk_wall - sk_irad, sk_floor_top])
                    cylinder(r=sk_irad, h=sk_tall_z);
                translate([-sk_inner_w/2, -10, sk_floor_top]) 
                    cube([sk_inner_w, 0.1, sk_tall_z]);
            }

            // C. Slice side walls
            sk_side_cutter();
        }

        // D. Round top corners
        sk_top_corner_rounder();
    }
}

// --- ASSEMBLY ---
module gps_tray() {
    difference() {
        // 1. The Soft Body
        minkowski() {
            skeleton();
            sphere(r=edge_rad, $fn=20); 
        }

        // 2. The Countersunk Mounting Holes
        for (x = [-mount_x_spacing/2, mount_x_spacing/2]) {
            translate([x, outer_l - mount_y_from_top, 0]) {
                translate([0, 0, -5]) 
                    cylinder(d=mount_hole_dia, h=20);
                translate([0, 0, floor - mount_head_depth]) 
                    cylinder(d1=mount_hole_dia, d2=mount_head_dia, h=mount_head_depth + 0.1);
            }
        }
        
        // 3. The Magnet Recess (Punched underneath the base)
        translate([0, mag_y_pos, -0.1])
            cylinder(d=mag_dia, h=mag_depth + 0.1);
    }
}

// --- RENDER ---
color("DarkCyan") gps_tray();