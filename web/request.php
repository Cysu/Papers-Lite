<?php

require_once('inc/common.php');
require_once 'inc/PapersLight.class.php';

session_start();
if (isset($_SESSION['pl'])) {
    $pl = unserialize($_SESSION['pl']);
} else {
    $pl = new PapersLight($type2attr);
}

if (isset($_GET['action'])) header('Content-Type: application/json');

if (isset($_GET['action'])) {
    if ($_GET['action'] === 'init') {
        echo json_encode(['username' => $pl->user]);
    }

    else if ($_GET['action'] === 'adminlogin' && isset($_POST['username']) && isset($_POST['password'])) {
        echo json_encode($pl->adminLogin($_POST['username'], $_POST['password']));
    }

    else if ($_GET['action'] === 'gettypes') {
        echo json_encode($pl->getTypes());
    }
    
    else if ($_GET['action'] === 'getpapers') {
        echo json_encode($pl->getPapers());
    }

    else if ($_GET['action'] === 'addpaper' && isset($_POST['type']) && isset($_POST['paper'])) {
        echo json_encode($pl->addPaper($_POST['type'], json_decode($_POST['paper'])));
    }
}
