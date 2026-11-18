<?php
require_once __DIR__ . '/config.php';

try {
    $pdo = get_db_connection();
    $sql = "
      SELECT b.bin_id, b.bin_name, b.location, b.full_threshold,
             r.distance_cm, r.fill_percent, r.recorded_at
      FROM bins b
      LEFT JOIN readings r ON r.reading_id = (
        SELECT r2.reading_id
        FROM readings r2
        WHERE r2.bin_id = b.bin_id
        ORDER BY r2.recorded_at DESC, r2.reading_id DESC
        LIMIT 1
      )
      ORDER BY b.bin_name
    ";
    $bins = $pdo->query($sql)->fetchAll();
} catch (Throwable $e) {
    http_response_code(500);
    $dbError = 'Could not connect to the database. Check config.php and import database/smart_bin.sql.';
    $bins = [];
}
function h($value): string {
    return htmlspecialchars((string)$value, ENT_QUOTES, 'UTF-8');
}
?>
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="5">
  <title>Smart Bin Management Dashboard</title>
  <link rel="stylesheet" href="assets/style.css">
</head>
<body>
<header class="header">
  <div class="wrap">
    <p class="eyebrow">IOT WASTE MONITORING</p>
    <h1>Smart Bin Management System</h1>
    <p>Monitor bin fill levels using ultrasonic sensor readings.</p>
  </div>
</header>
<main class="wrap">
  <?php if (isset($dbError)): ?>
    <div class="notice error"><?= h($dbError) ?></div>
  <?php endif; ?>
  <?php
    $total = count($bins);
    $full = count(array_filter($bins, fn($b) => $b['fill_percent'] !== null && (int)$b['fill_percent'] >= (int)$b['full_threshold']));
    $active = count(array_filter($bins, fn($b) => $b['fill_percent'] !== null));
  ?>
  <section class="summary">
    <article class="metric"><span>Total bins</span><strong><?= $total ?></strong></article>
    <article class="metric"><span>Reporting bins</span><strong><?= $active ?></strong></article>
    <article class="metric"><span>Full bins</span><strong><?= $full ?></strong></article>
  </section>

  <section class="panel">
    <div class="panel-heading">
      <div><h2>Bin status</h2><p>Dashboard refreshes every 5 seconds.</p></div>
      <span class="live"><i></i> Live dashboard</span>
    </div>
    <?php if (!$bins): ?>
      <p class="empty">No bins found. Import the database SQL file to create the sample bin.</p>
    <?php else: ?>
      <div class="grid">
      <?php foreach ($bins as $bin):
        $hasReading = $bin['fill_percent'] !== null;
        $percent = $hasReading ? max(0, min(100, (int)$bin['fill_percent'])) : 0;
        $isFull = $hasReading && $percent >= (int)$bin['full_threshold'];
        $status = !$hasReading ? 'Waiting for data' : ($isFull ? 'FULL' : 'Normal');
      ?>
        <article class="bin-card">
          <div class="card-top">
            <div><h3><?= h($bin['bin_name']) ?></h3><p><?= h($bin['location']) ?> · <?= h($bin['bin_id']) ?></p></div>
            <span class="pill <?= $isFull ? 'full' : ($hasReading ? 'normal' : 'waiting') ?>"><?= h($status) ?></span>
          </div>
          <div class="level-row"><span>Estimated fill level</span><strong><?= $hasReading ? $percent . '%' : '—' ?></strong></div>
          <div class="track"><div class="fill <?= $isFull ? 'fill-full' : '' ?>" style="width: <?= $percent ?>%"></div></div>
          <div class="details">
            <span>Distance: <?= $hasReading ? h($bin['distance_cm']) . ' cm' : 'No reading' ?></span>
            <span>Threshold: <?= h($bin['full_threshold']) ?>%</span>
          </div>
          <p class="updated">Last update: <?= $hasReading ? h($bin['recorded_at']) : 'Waiting for ESP8266 data' ?></p>
        </article>
      <?php endforeach; ?>
      </div>
    <?php endif; ?>
  </section>
  <p class="footnote">Fill level is estimated from sensor distance. Calibrate the sensor and bin dimensions for reliable readings.</p>
</main>
<footer>Smart Bin Management System · IoT prototype</footer>
</body>
</html>
