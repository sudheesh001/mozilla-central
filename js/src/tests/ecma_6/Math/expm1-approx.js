assertNear(Math.expm1(1e-300), 1e-300);
assertNear(Math.expm1(1e-100), 1e-100);
assertNear(Math.expm1(1e-14), 1.000000000000005e-14);
assertNear(Math.expm1(1e-6), 0.0000010000005000001665);

var expm1_data = [
    [ -1.875817529344e-70, -1.875817529344e-70 ],
    [ -7.09962844069878e-15, -7.099628440698755e-15 ],
    [ -2.114990849122478e-10, -2.1149908488988187e-10 ],
    [ -0.0000031404608812881633, -0.000003140455950046052 ],

    [ 1.875817529344e-70, 1.875817529344e-70 ],
    [ 6.261923313140869e-30, 6.261923313140869e-30 ],
    [ 7.09962844069878e-15, 7.099628440698805e-15 ],
    [ 1.3671879628418538e-12, 1.3671879628427884e-12 ],
    [ 2.114990849122478e-10, 2.1149908493461373e-10 ],
    [ 1.6900931765206906e-8, 1.6900931908027652e-8 ],
    [ 0.0000031404608812881633, 0.0000031404658125405988 ]
];

for (let [x, y] of expm1_data)
    assertNear(Math.expm1(x), y);

reportCompare(0, 0, "ok");
