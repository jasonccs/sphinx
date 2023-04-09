<?php

require ( "sphinxapi.php" );
$sp = new SphinxClient();


$sp->SetServer('127.0.0.1',9312);
$sp->SetConnectTimeout ( 5 );
$sp->SetArrayResult ( true );
//$sp->SetFilter('id',[2,1]); // 对 matches 的 主键的过滤查询
//$res = $sp->Query ( '汉', 'test1' ); // 对 所有的 fields 字段的查询
//$res = $sp->Query ( '武汉', 'test1:testrt' ); // 对 定义中的多个index 指定 fields 字段 查询
//$res = $sp->Query ( '@title 湖北', 'test1' ); // 对 指定 fields 字段 查询
$res = $sp->Query ( '@con 湖北', 'test1' ); // 对 指定 fields 字段 查询
echo '<pre>';
var_dump($res);