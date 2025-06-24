package com.jumprun.game;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;

public class Player {
    public float x, y;
    public int width, height;
    private float velocityY;
    private float gravity = 0.8f;
    private float jumpStrength = -18f;
    private boolean isOnGround;
    private int groundLevel;

    public Player(float x, float y, int width, int height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.velocityY = 0;
        this.isOnGround = true;
    }

    public void update(int screenHeight) {
        groundLevel = screenHeight - 100 - height;
        
        // Apply gravity
        if (!isOnGround) {
            velocityY += gravity;
        }
        
        // Update position
        y += velocityY;
        
        // Check ground collision
        if (y >= groundLevel) {
            y = groundLevel;
            velocityY = 0;
            isOnGround = true;
        } else {
            isOnGround = false;
        }
    }

    public void jump() {
        if (isOnGround) {
            velocityY = jumpStrength;
            isOnGround = false;
        }
    }

    public void draw(Canvas canvas, Paint paint) {
        paint.setColor(Color.BLUE);
        canvas.drawRect(x, y, x + width, y + height, paint);
        
        // Draw simple face
        paint.setColor(Color.WHITE);
        canvas.drawCircle(x + width * 0.3f, y + height * 0.3f, 5, paint);
        canvas.drawCircle(x + width * 0.7f, y + height * 0.3f, 5, paint);
        
        paint.setColor(Color.RED);
        canvas.drawRect(x + width * 0.3f, y + height * 0.6f, x + width * 0.7f, y + height * 0.7f, paint);
    }

    public Rect getBounds() {
        return new Rect((int)x, (int)y, (int)(x + width), (int)(y + height));
    }
}
