
//3.42
SELECT codpro, SUM(cantidad)
    FROM ventas GROUP BY codpro
    HAVING
        SUM(cantidad) > ( SELECT SUM(cantidad) FROM ventas v1
                                WHERE v1.codpro = 'S1' GROUP BY v1.codpro );
SELECT codpro, SUM(cantidad)
    FROM ventas WHERE codpro='S1'
    GROUP BY codpro;

//3.43, pongo los 3 mejores
SELECT v.codpro, SUM(v.cantidad) total
    FROM ventas v GROUP BY v.codpro
    HAVING 2 >= ( COUNT(*) FROM ( SELECT v1.codpro FROM ventas v1 GROUP BY v1.codpro
                        HAVING SUM(v1.cantidad) > total));

//3.44
SELECT DISTINCT v.codpro FROM ventas v 
WHERE NOT EXISTS (
    (SELECT p1.ciudad FROM proyecto p1, ventas v1 
    WHERE p1.codpj = v1.codpj AND v1.codpro = 'S3') 
    MINUS 
    (SELECT p2.ciudad FROM proyecto p2, ventas v2
    WHERE p2.codpj = v2.codpj AND v2.codpro = v.codpro)
);

//3.45
SELECT codpro, COUNT(*) FROM ventas
GROUP BY codpro
HAVING COUNT(*) >= 10;

//3.46
SELECT DISTINCT v.codpro FROM ventas v
WHERE NOT EXISTS (
    (SELECT DISTINCT v1.codpie FROM ventas v1
        WHERE v1.codpro = 'S1' )
    MINUS
    (SELECT DISTINCT v2.codpie FROM ventas v2
        WHERE v2.codpro = v.codpro)
);
// Para comprobar
SELECT DISTINCT codpro, codpie FROM ventas ORDER BY codpro, codpie;

//3.47
SELECT v.codpro, SUM(cantidad) FROM ventas v
WHERE NOT EXISTS (
    (SELECT DISTINCT v1.codpie FROM ventas v1
        WHERE v1.codpro = 'S1' )
    MINUS
    (SELECT DISTINCT v2.codpie FROM ventas v2
        WHERE v2.codpro = v.codpro)
)
GROUP BY v.codpro;

//3.48
SELECT * FROM proyecto p
    WHERE NOT EXISTS (
    (SELECT v1.codpro FROM ventas v1
        WHERE v1.codpie = 'P3')
    MINUS
    (SELECT v2.codpro FROM ventas v2
        WHERE v2.codpj = p.codpj)
);
// Comprobacion manual
SELECT DISTINCT codpj, codpro FROM ventas ORDER BY codpj, codpro;
SELECT DISTINCT codpro FROM ventas WHERE codpie = 'P3';

//3.49
SELECT v.codpro, AVG(v.cantidad) FROM ventas v
WHERE EXISTS ( SELECT v1.codpro FROM ventas v1 WHERE v1.codpie = 'P3' AND
                                                    v1.codpro = v.codpro )
GROUP BY v.codpro;

//3.50
SELECT index_name,table_name,table_owner FROM USER_INDEXES;

//3.51
SELECT * FROM DICTIONARY where table_name like '%USER%';
SELECT * FROM ALL_USERS ORDER BY created desc;
SELECT * FROM PARTICIPA;
// MAL

//3.52
SELECT codpro,TO_CHAR(fecha, 'YYYY') anio, AVG(cantidad) media FROM ventas
GROUP BY codpro,TO_CHAR(fecha, 'YYYY')
ORDER BY codpro,TO_CHAR(fecha, 'YYYY');

//3.53
SELECT codpro FROM ventas v, pieza p
    WHERE v.codpie = p.codpie AND p.color = 'Rojo'
GROUP BY codpro ORDER BY codpro;

//3.54
SELECT * FROM proveedor p WHERE NOT EXISTS(
(SELECT codpie FROM pieza WHERE color = 'Rojo')
MINUS
(SELECT codpie FROM ventas v1 WHERE v1.codpro = p.codpro)
) ORDER BY codpro;
// Comprobacion
SELECT DISTINCT codpro,codpie FROM ventas NATURAL JOIN pieza WHERE color='Rojo';

//3.55
SELECT * FROM proveedor p WHERE NOT EXISTS (
( SELECT * FROM ventas NATURAL JOIN pieza WHERE codpro=p.codpro 
                                            AND NOT color='Rojo'));
//Comprobacion manual                                            
SELECT DISTINCT codpro,codpie FROM ventas NATURAL JOIN pieza
ORDER BY codpro,codpie;

//3.56
SELECT * FROM proveedor p WHERE 1 < ( SELECT COUNT(*) FROM ventas v NATURAL JOIN pieza
    WHERE color = 'Rojo' AND p.codpro=v.codpro GROUP BY v.codpro);
// Comprobacion
SELECT codpro,codpie,color FROM ventas NATURAL JOIN pieza
WHERE color = 'Rojo' ORDER BY codpro,codpie;

//3.57
SELECT * FROM proveedor WHERE codpro IN
    (SELECT codpro FROM ventas v WHERE NOT EXISTS (
        (SELECT codpie FROM pieza WHERE color = 'Rojo')
        MINUS
        (SELECT v1.codpie FROM ventas v1 WHERE v1.codpro=v.codpro))
    GROUP BY codpro HAVING SUM(cantidad)>10)
ORDER BY codpro;

//3.58
UPDATE proveedor p
    SET status = 1
    WHERE NOT EXISTS (
        SELECT * FROM ventas WHERE NOT codpie = 'P1' AND codpro=p.codpro );
//Comprobacion
SELECT * FROM proveedor;
SELECT DISTINCT codpro,codpie FROM ventas ORDER BY codpro,codpie;

//3.59
SELECT codpie, ciudad FROM (
    SELECT * FROM pieza p, ventas v WHERE p.codpie = v.codpie AND NOT EXISTS (
        SELECT * FROM ventas WHERE codpie=p.codpie AND
            TO_CHAR(fecha, 'mm/yyyy') = '09/2009' )
    ) v
GROUP BY codpie HAVING
    SUM(cantidad WHERE TO_CHAR(fecha, 'mm/yyyy') = '08/2009') >= 
        MAX(SELECT SUM( SELECT cantidad FROM ventas v2 WHERE
                 TO_CHAR(fecha, 'mm/yyyy') = '08/2009') FROM ventas
                 GROUP BY codpie HAVING codpie IN (SELECT codpie FROM v));
    
                                            