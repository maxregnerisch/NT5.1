package com.jumprun.game;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

public class GameView extends SurfaceView implements SurfaceHolder.Callback {
    private GameThread gameThread;
    private Player player;
    private ArrayList<Coin> coins;
    private ArrayList<Enemy> enemies;
    private Paint paint;
    private Random random;
    private int screenWidth, screenHeight;
    private int score = 0;
    private long lastCoinSpawn = 0;
    private long lastEnemySpawn = 0;

    public GameView(Context context) {
        super(context);
        getHolder().addCallback(this);
        
        paint = new Paint();
        random = new Random();
        coins = new ArrayList<>();
        enemies = new ArrayList<>();
        
        setFocusable(true);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        screenWidth = getWidth();
        screenHeight = getHeight();
        
        player = new Player(100, screenHeight - 200, 60, 80);
        
        gameThread = new GameThread(getHolder(), this);
        gameThread.setRunning(true);
        gameThread.start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        boolean retry = true;
        gameThread.setRunning(false);
        while (retry) {
            try {
                gameThread.join();
                retry = false;
            } catch (InterruptedException e) {
                // Will try again
            }
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            player.jump();
        }
        return true;
    }

    public void update() {
        player.update(screenHeight);
        
        // Spawn coins
        if (System.currentTimeMillis() - lastCoinSpawn > 2000) {
            coins.add(new Coin(screenWidth, random.nextInt(screenHeight - 200) + 100, 30));
            lastCoinSpawn = System.currentTimeMillis();
        }
        
        // Spawn enemies
        if (System.currentTimeMillis() - lastEnemySpawn > 3000) {
            enemies.add(new Enemy(screenWidth, random.nextInt(screenHeight - 200) + 100, 
                       40 + random.nextInt(40), random.nextFloat() * 3 + 1));
            lastEnemySpawn = System.currentTimeMillis();
        }
        
        // Update coins
        Iterator<Coin> coinIterator = coins.iterator();
        while (coinIterator.hasNext()) {
            Coin coin = coinIterator.next();
            coin.update();
            
            // Check collision with player
            if (player.getBounds().intersect(coin.getBounds())) {
                score += 10;
                coinIterator.remove();
            } else if (coin.x < -coin.radius) {
                coinIterator.remove();
            }
        }
        
        // Update enemies
        Iterator<Enemy> enemyIterator = enemies.iterator();
        while (enemyIterator.hasNext()) {
            Enemy enemy = enemyIterator.next();
            enemy.update();
            
            // Check collision with player
            if (player.getBounds().intersect(enemy.getBounds())) {
                // Game over or damage logic here
                score = Math.max(0, score - 5);
            }
            
            if (enemy.x < -enemy.radius) {
                enemyIterator.remove();
            }
        }
    }

    public void draw(Canvas canvas) {
        if (canvas != null) {
            // Clear screen
            canvas.drawColor(Color.rgb(135, 206, 235)); // Sky blue
            
            // Draw ground
            paint.setColor(Color.rgb(34, 139, 34));
            canvas.drawRect(0, screenHeight - 100, screenWidth, screenHeight, paint);
            
            // Draw player
            player.draw(canvas, paint);
            
            // Draw coins
            paint.setColor(Color.YELLOW);
            for (Coin coin : coins) {
                coin.draw(canvas, paint);
            }
            
            // Draw enemies
            for (Enemy enemy : enemies) {
                enemy.draw(canvas, paint);
            }
            
            // Draw score
            paint.setColor(Color.BLACK);
            paint.setTextSize(60);
            canvas.drawText("Score: " + score, 50, 100, paint);
        }
    }

    public void resume() {
        if (gameThread != null) {
            gameThread.setRunning(true);
        }
    }

    public void pause() {
        if (gameThread != null) {
            gameThread.setRunning(false);
        }
    }
}
