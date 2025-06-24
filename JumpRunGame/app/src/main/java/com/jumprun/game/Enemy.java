package com.jumprun.game;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import java.util.Random;

public class Enemy {
    public float x, y;
    public int radius;
    private float speed;
    private float baseSpeed;
    private Random random;
    private int color;
    private float targetY;
    private float verticalSpeed = 0;
    private long lastDirectionChange = 0;
    private float pulsation = 0;

    public Enemy(float x, float y, int radius, float speed) {
        this.x = x;
        this.y = y;
        this.radius = radius;
        this.speed = speed;
        this.baseSpeed = speed;
        this.random = new Random();
        this.targetY = y;
        
        // Random colors like agar.io
        int[] colors = {
            Color.rgb(255, 100, 100), // Red
            Color.rgb(100, 255, 100), // Green
            Color.rgb(100, 100, 255), // Blue
            Color.rgb(255, 255, 100), // Yellow
            Color.rgb(255, 100, 255), // Magenta
            Color.rgb(100, 255, 255), // Cyan
            Color.rgb(255, 150, 100), // Orange
            Color.rgb(150, 100, 255)  // Purple
        };
        this.color = colors[random.nextInt(colors.length)];
    }

    public void update() {
        // Move left (main movement)
        x -= speed;
        
        // Agar.io style floating movement
        pulsation += 0.1f;
        
        // Change direction occasionally
        if (System.currentTimeMillis() - lastDirectionChange > 1000 + random.nextInt(2000)) {
            targetY = y + (random.nextFloat() - 0.5f) * 200;
            lastDirectionChange = System.currentTimeMillis();
        }
        
        // Move towards target Y with smooth interpolation
        float deltaY = targetY - y;
        verticalSpeed = deltaY * 0.02f;
        y += verticalSpeed;
        
        // Add slight pulsation effect like agar.io
        float pulsationOffset = (float)Math.sin(pulsation) * 2;
        
        // Vary speed slightly for organic movement
        speed = baseSpeed + (float)Math.sin(pulsation * 0.5f) * 0.5f;
    }

    public void draw(Canvas canvas, Paint paint) {
        // Draw main body with pulsation effect
        float currentRadius = radius + (float)Math.sin(pulsation) * 3;
        
        // Draw shadow/outline
        paint.setColor(Color.rgb(50, 50, 50));
        canvas.drawCircle(x + 3, y + 3, currentRadius + 2, paint);
        
        // Draw main body
        paint.setColor(color);
        canvas.drawCircle(x, y, currentRadius, paint);
        
        // Draw inner highlight
        paint.setColor(Color.WHITE);
        paint.setAlpha(100);
        canvas.drawCircle(x - currentRadius * 0.3f, y - currentRadius * 0.3f, currentRadius * 0.4f, paint);
        paint.setAlpha(255);
        
        // Draw simple face or pattern
        paint.setColor(Color.BLACK);
        // Eyes
        canvas.drawCircle(x - currentRadius * 0.3f, y - currentRadius * 0.2f, 4, paint);
        canvas.drawCircle(x + currentRadius * 0.3f, y - currentRadius * 0.2f, 4, paint);
        
        // Mouth (changes with pulsation)
        float mouthSize = 3 + (float)Math.sin(pulsation) * 2;
        canvas.drawCircle(x, y + currentRadius * 0.3f, mouthSize, paint);
        
        // Draw spikes or texture around the edge
        paint.setColor(getDarkerColor(color));
        for (int i = 0; i < 8; i++) {
            float angle = (float)(i * Math.PI * 2 / 8 + pulsation * 0.1);
            float spikeX = x + (float)Math.cos(angle) * currentRadius * 0.8f;
            float spikeY = y + (float)Math.sin(angle) * currentRadius * 0.8f;
            canvas.drawCircle(spikeX, spikeY, 3, paint);
        }
    }
    
    private int getDarkerColor(int color) {
        int red = (Color.red(color) * 3) / 4;
        int green = (Color.green(color) * 3) / 4;
        int blue = (Color.blue(color) * 3) / 4;
        return Color.rgb(red, green, blue);
    }

    public Rect getBounds() {
        return new Rect((int)(x - radius), (int)(y - radius), 
                       (int)(x + radius), (int)(y + radius));
    }
}
