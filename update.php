<?php
declare(strict_types=1);
header('Content-Type: application/json; charset=utf-8');

require_once __DIR__ . '/../config.php';

function respond(int $status, array $payload): void {
    http_response_code($status);
    echo json_encode($payload);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    respond(405, ['success' => false, 'message' => 'POST method required']);
}

$token = (string)($_POST['token'] ?? '');
$binId = trim((string)($_POST['bin_id'] ?? ''));
$distance = filter_var($_POST['distance_cm'] ?? null, FILTER_VALIDATE_FLOAT);
$fill = filter_var($_POST['fill_percent'] ?? null, FILTER_VALIDATE_INT);

if (!hash_equals(API_TOKEN, $token) || API_TOKEN === 'CHANGE_THIS_TO_A_LONG_RANDOM_SECRET') {
    respond(401, ['success' => false, 'message' => 'Invalid or unconfigured API token']);
}

if ($binId === '' || strlen($binId) > 40 || $distance === false || $fill === false ||
    $distance < 0 || $distance > 1000 || $fill < 0 || $fill > 100) {
    respond(400, ['success' => false, 'message' => 'Invalid reading data']);
}

try {
    $pdo = get_db_connection();
    $check = $pdo->prepare('SELECT bin_id FROM bins WHERE bin_id = ?');
    $check->execute([$binId]);
    if (!$check->fetch()) {
        respond(404, ['success' => false, 'message' => 'Unknown bin_id']);
    }

    $insert = $pdo->prepare(
        'INSERT INTO readings (bin_id, distance_cm, fill_percent) VALUES (?, ?, ?)'
    );
    $insert->execute([$binId, $distance, $fill]);

    respond(200, ['success' => true, 'message' => 'Reading saved']);
} catch (Throwable $e) {
    error_log($e->getMessage());
    respond(500, ['success' => false, 'message' => 'Server error']);
}
