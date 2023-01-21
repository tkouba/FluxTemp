

/*[Hidden]*/

// Wemos/Lolin D1 mini size
miniX = 26.0; // 25.6 mm
miniY = 34.6; // 34.2 mm
miniZ1 = 5.5; // Above center of USB
miniZ2 = 2.2; // Under center of USB
miniR = 1.5;  // Corner round

// USB connector size
usbX = 12.0;
usbY = 30.0;
usbZ = 8.0; // Connector is symmetrical
usbD = 4.0; // USB cable dmeter

// DHT11 module size
dhtX1 = 14.0;
dhtY1 = 29.0;
dhtOf = 0.2; // Offset of sensor/pcb
// DHT11 sensor size
dhtX = 12.2; // 12.0 mm
dhtY = 15.7; // 15.5 mm
dhtZ = 5.5;  // 5.5 mm
// Space between DHT11 module and D1 mini
dhtSpace = 15.0;

// Screw (3.0x10)
screwHD1 = 5.8; // Screw head diameter 1
screwHD2 = 3.2; // Screw head diameter 2
screwHk = 1.5;  // Screw head head length
screwDia = 2.8; // Screw hole diameter

wt = 1.8;  // wall thickness
st = 0.1;  // small thickness
ht = 0.66; // honeycomb thickness

// case
holeR = 4; // case montage hole R
caseR = 3;
caseX = 40;
caseY = 2*usbY+miniY+dhtSpace+dhtY1+4*wt+caseR+2*holeR;
caseZ = usbZ/2+miniZ1+4*wt+st;

// Better rounds
$fn = $preview ? $fn : 64; 

echo("Case Y", caseY);
echo("Case Z", caseZ);
translate([2*caseX,0,0]) front();
back();

module case() {  
  translate([-caseX/2,0,0])
    hull() {
      translate([caseR,st,caseR])
        rotate([-90,0,0]) cylinder(r=caseR,h=caseR);
      translate([caseX-caseR,st,caseR])
        rotate([-90,0,0]) cylinder(r=caseR,h=caseR);
      translate([caseR,caseY-caseR,caseR])
        sphere(r=caseR);
      translate([caseX-caseR,caseY-caseR,caseR])
        sphere(r=caseR);
      translate([0,st,caseZ-caseR])
        cube(caseR);
      translate([caseX-caseR,st,caseZ-caseR])
        cube(caseR);
      translate([caseR,caseY-caseR,caseZ-caseR])
        cylinder(r=caseR,h=caseR);
      translate([caseX-caseR,caseY-caseR,caseZ-caseR])
        cylinder(r=caseR,h=caseR);
    }  
}

module back() {
  difference() {
    union() {
      hull() {
        translate([-caseX/2+wt+st,usbY/2+wt+st,0])
          cube(wt);
        translate([caseX/2-2*wt-st,usbY/2+wt+st,0])
          cube(wt);
        translate([-caseX/2+wt+st,caseY-4*wt-caseR-2*holeR,0])
          cube(wt);
        translate([caseX/2-2*wt-st,caseY-4*wt-caseR-2*holeR,0])
          cube(wt);
      }
      translate([-usbD/2+st,usbY/2+wt+st,0])
        cube([usbD-2*st,usbY/2-st,miniZ1+2*wt]);
    }
    translate([0,usbY/4,miniZ1+2*wt])
      rotate([-90,0,0])
         cylinder(h=usbY,d=usbD);
    translate([-caseX/2+wt+screwHD1,usbY/2+wt+screwHD1,0])
      screwHead(2*wt);
    translate([caseX/2-wt-screwHD1,usbY/2+wt+screwHD1,0])
      screwHead(2*wt);
    translate([-caseX/2+wt+screwHD1,caseY-4*wt-caseR-2*holeR-screwHD1,0])
      screwHead(2*wt);
    translate([caseX/2-wt-screwHD1,caseY-4*wt-caseR-2*holeR-screwHD1,0])
      screwHead(2*wt);
  }
}

module front() {  
  difference() {
    case();
    union() {
      translate([0,0,wt]) {
        complete();
        translate([0,0,usbZ/2])
          linear_extrude(miniZ1+3*wt+2*st)
            projection() complete();
      }    
    }
    hull() {
      translate([-caseX/2+wt,usbY/2+wt,caseZ-wt-st])
        cube(wt+2*st);
      translate([caseX/2-2*wt-2*st,usbY/2+wt,caseZ-wt-st])
        cube(wt+2*st);
      translate([-caseX/2+wt,caseY-4*wt-caseR-2*holeR-2*st,caseZ-wt-st])
        cube(wt+2*st);
      translate([caseX/2-2*wt-2*st,caseY-4*wt-caseR-2*holeR-2*st,caseZ-wt-st])
        cube(wt+2*st);
    }
    translate([-caseX/2+wt+screwHD1,usbY/2+wt+screwHD1,3*wt])
      cylinder(d=screwDia,h=caseZ);
    translate([caseX/2-wt-screwHD1,usbY/2+wt+screwHD1,3*wt])
      cylinder(d=screwDia,h=caseZ);
    translate([-caseX/2+wt+screwHD1,caseY-4*wt-caseR-2*holeR-screwHD1,3*wt])
      cylinder(d=screwDia,h=caseZ);
    translate([caseX/2-wt-screwHD1,caseY-4*wt-caseR-2*holeR-screwHD1,3*wt])
      cylinder(d=screwDia,h=caseZ);
    translate([0,caseY-2*wt-caseR-holeR,caseZ-0.7*holeR])
      sphere(r=holeR);
    translate([0,caseY-2*wt-caseR-holeR,3*wt])
      cylinder(r=holeR,h=caseZ-0.7*holeR-3*wt);
  }
}

module complete() {  
  union() {
    mini_with_usb();
    translate([0,2*usbY+miniY+dhtSpace])
      dht11_module();
    translate([-dhtX1*.9/2,2*usbY+miniY,dhtZ/2])
      cube([dhtX1*.9,dhtSpace,dhtZ/2]);
  }
}

module mini_with_usb() {
  translate([0,0,usbZ/2]) {
    translate([-miniX/2,2*usbY,-miniZ2]) {    
      linear_extrude(miniZ1+miniZ2) {
        hull() {
          square(miniR);
          translate([miniX-miniR,0])
            square(miniR);
          translate([miniR,miniY-miniR])
            circle(miniR);
          translate([miniX-miniR,miniY-miniR])
            circle(miniR);
        }
      }
    }
    translate([-usbX/2,usbY,-usbZ/2])
      cube([usbX, usbY, usbZ]);
    rotate([-90,0,0])
      cylinder(h=usbY,d=usbD);    
  }
}

module dht11_module() {
  union() {
    translate([-dhtX1/2,0,0])
      cube([dhtX1,dhtY1,dhtZ]);
    translate([(dhtX1-dhtX)/2-dhtX1/2,dhtY1-dhtOf-dhtY,-wt-.1])
      linear_extrude(wt+.1)
        honeycomb(dhtX,dhtY,2*ht,ht);
  }
}

/* parametric honeycomb holes
    x - 
    y
    d - hole diameter
    wt - wall thickness
 */
module honeycomb(x, y, d, wt) {
  xStep = d*3/2 + wt*cos(30)*2;
  xStart =(((ceil(x/xStep)*xStep)-x)/2); // centering X
  yStep = d*cos(30) + wt;
  yStart = (((ceil(y/yStep)*yStep)-y)/2); // centering Y
  intersection()  {
    square([x, y]);
    // Note, number of step+1 to ensure the whole surface is covered
    for (yOffset = [-yStart:yStep:y+yStep], xOffset = [-xStart:xStep:x+xStart]) {
      translate([xOffset, yOffset]) {
        circle(d=d, $fn=6);
      }
      translate([xOffset + d*3/4 + wt*cos(30), yOffset + (d*cos(30)+wt)/2]) {
        circle(d=d, $fn=6);
      }
    }
  }    
} 

module screwHead(length) {
  union() {
    cylinder(h=screwHk,d1=screwHD1,d2=screwHD2);
    cylinder(h=length,d=screwHD2);
  }
}
