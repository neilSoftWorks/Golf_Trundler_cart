// --- THUMBSCREW KNOB FOR 3mm SCREW ---

// --- 1. KNOB PARAMETERS ---
knob_dia = 16.0;       // Outer diameter of the knob
knob_height = 10.0;    // Total height of the knob
top_chamfer = 1.5;     // Chamfer on the top edge for thumb comfort

// --- 2. GRIP (FLUTED KNURLING) ---
grip_count = 12;       // Number of grip indents around the edge
grip_dia = 3.5;        // Diameter of the grip cutouts

// --- 3. SCREW DIMENSIONS (3x18mm Self Tapping) ---
// Note: Standard 3mm pan heads are usually ~5.5mm. 
// Set to 6.2mm to allow clearance for easy insertion.
head_dia = 6.2;        // Diameter of the top pocket for the screw head
head_depth = 3.0;      // Depth of the top pocket 
shaft_dia = 2.9;       // Clearance hole for the 3mm threads to pass through

$fn = 60;

// --- MAIN ASSEMBLY ---
difference() {
    
    // 1. The main chamfered knob body
    rotate_extrude() {
        polygon([
            [0, 0],
            [knob_dia/2, 0],
            [knob_dia/2, knob_height - top_chamfer],
            [(knob_dia/2) - top_chamfer, knob_height],
            [0, knob_height]
        ]);
    }
    
    // 2. Cutouts for the fluted grip
    for(i = [0 : grip_count - 1]) {
        rotate([0, 0, i * (360 / grip_count)])
            translate([knob_dia / 2, 0, -1])
            cylinder(d=grip_dia, h=knob_height + 2, $fn=30);
    }
    
    // 3. Cutouts for the screw
    // Shaft hole through the center
    translate([0, 0, -1]) 
        cylinder(d=shaft_dia, h=knob_height + 2);
        
    // Counterbore pocket for the screw head (Insert from Top)
    translate([0, 0, knob_height - head_depth + 0.01]) 
        cylinder(d=head_dia, h=head_depth + 1);
}