// --- PARAMETERS ---

// 1. Inner Dimensions (To fit the GPS)
inner_w = 35.0; // Width (X-axis)
inner_l = 41.0; // Length (Y-axis)

// 2. Heights
tall_z  = 18.0; // Top and Bottom wall height
short_z = 8.0;  // Left and Right wall height

// 3. Wall Settings
wall = 1.6;     // Wall thickness
floor = 1.6;    // Base thickness
rad = 2.0;      // Outer corner rounding radius

// 4. The Sweep!
trans_rad = 4.0; // Radius of the curved join between short and tall edges

// --- CALCULATIONS ---
outer_w = inner_w + (wall * 2);
outer_l = inner_l + (wall * 2);
$fn = 60; // Smoothness

// --- MODULES ---

// Helper to make rounded boxes
module rounded_box(w, l, h, r) {
    hull() {
        for(x=[-1,1], y=[-1,1]) {
            translate([x*(w/2 - r), y*(l/2 - r), 0])
                cylinder(r=r, h=h);
        }
    }
}

// The Cover
module gps_cover() {
    difference() {
        // 1. The Solid Outer Box (Tall)
        rounded_box(outer_w, outer_l, tall_z, rad);

        // 2. Hollow out the inside
        translate([0, 0, floor])
            rounded_box(inner_w, inner_l, tall_z + 1, max(0.1, rad - wall));

        // 3. Slice the sides down with a smooth fillet
        // Instead of a simple cube, we use a custom hull to carve the swoop
        hull() {
            // The two curved bottom edges of the cutter
            for (y = [-1, 1]) {
                translate([0, y * (inner_l/2 - trans_rad), short_z + trans_rad])
                    rotate([0, 90, 0])
                    cylinder(r=trans_rad, h=outer_w + 10, center=true);
            }
            // A block at the top to ensure everything above the curve is deleted
            translate([0, 0, tall_z + 10])
                cube([outer_w + 10, inner_l, 0.1], center=true);
        }
    }
}

// --- RENDER ---
color("DarkCyan") gps_cover();