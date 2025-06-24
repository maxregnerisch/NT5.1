package com.jumprun.game;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;

public class Coin {
    public float x, y;
    public int radius;
    private float speed = 5f;
    private float rotation = 0;

    public Coin(float x, float y, int radius) {
        this.x = x;
        this.y = y;
        this.radius = radius;
    }

    public void update() {
        x -= speed;
        rotation += 5; // Spinning effect
        if (rotation >= 360) rotation = 0;
    }

    public void draw(Canvas canvas, Paint paint) {
        // Draw coin with spinning effect
        paint.setColor(Color.YELLOW);
        canvas.drawCircle(x, y, radius, paint);
        
        // Draw inner circle for depth
        paint.setColor(Color.rgb(255, 215, 0)); // Gold color
        canvas.drawCircle(x, y, radius * 0.7f, paint);
        
        // Draw dollar sign or simple pattern
        paint.setColor(Color.rgb(184, 134, 11)); // Darker gold
        paint.setStrokeWidth(4);
        canvas.drawLine(x - radius * 0.3f, y - radius * 0.4f, x + radius * 0.3f, y + radius * 0.4f, paint);
        canvas.drawLine(x - radius * 0.3f, y + radius * 0.4f, x + radius * 0.3f, y - radius * 0.4f, paint);
    }

    public Rect getBounds() {
        return new Rect((int)(x - radius), (int)(y - radius), 
                       (int)(x + radius), (int)(y + radius));
    }
}
