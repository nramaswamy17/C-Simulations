#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

class Block {
    public: 
        float x, y;
        float width, height;
        float vx, vy;
        float mass;
        sf::Color color;

    Block(float px, float py, float w, float h, float m, float velx, sf::Color c) {
        x = px; y = py;
        width = w; height = h;
        mass = m;
        vx = velx; vy = 0;
        color = c;
    }

    void update(float dt) {
        x += vx * dt;
        y += vy * dt;
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(color);
        rect.setOutlineThickness(2);
        rect.setOutlineColor(sf::Color::Black);
        window.draw(rect);

        //draw velocity arrows
        if (vx != 0) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x + width / 2, y + height / 2)),
                sf::Vertex(sf::Vector2f(x + width / 2 + vx + .5f, y + height/2))
            };
            line[0].color = color;
            line[1].color = color;
            window.draw(line, 2, sf::Lines);
        }
    }

    bool collidesWith(Block& other) {
        return (x < other.x + other.width && 
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }

    void bounceOffWalls(float windowWidth){
        if (x <= 0 || x + width > windowWidth) {
            vx *= -1;
            x = (x <= 0) ? 0 : windowWidth - width;
        }
    }  
};

void handleCollision(Block& b1, Block& b2) {
    float m1 = b1.mass;
    float m2 = b2.mass;
    float v1 = b1.vx;
    float v2 = b2.vx;

    // Elastic collision formulas
    b1.vx = ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
    b2.vx = ((m1 - m2) * v2 + 2 * m1 * v1) / (m1 + m2);

    // Separate blocks to prevent overlap
    float overlap = (b1.x + b1.width) - b2.x;
    b1.x -= overlap / 2;
    b2.x += overlap / 2;
}

int main() {
        // Create window
    sf::RenderWindow window(sf::VideoMode(800, 600), "2D Block Collision Simulator");
    window.setFramerateLimit(60);
    
    // Load font for text
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        // Try Windows font path
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            // Try Linux font path
            if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                std::cout << "Warning: Could not load font. Text will not display." << std::endl;
            }
        }
    }

    // Create blocks
    Block block1(100, 250, 80, 100, 2.0f, 150.0f, sf::Color(49, 130, 206));
    Block block2(600, 250, 80, 100, 1.0f, -100.0f, sf::Color(229, 62, 62));

    bool collisionHappened = false;
    sf::Clock clock;
    sf::Clock collisionTimer;
    bool showCollisionText = false;
    bool showEnergyText = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        
            // Reset on R key
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                    Block block1(100, 250, 80, 100, 2.0f, 150.0f, sf::Color(49, 130, 206));
                    Block block2(600, 250, 80, 100, 1.0f, -100.0f, sf::Color(229, 62, 62));
                    bool collisionHappened = true;
                    bool showCollisionText = true;
                    collisionTimer.restart();
            }
        }

        // Update physics
        float dt = clock.restart().asSeconds();
        block1.update(dt);
        block2.update(dt);

        // Check collision
        if (block1.collidesWith(block2) && !collisionHappened){
            handleCollision(block1, block2);
            collisionHappened = true;
            showCollisionText = true;
            collisionTimer.restart();
        }

        // Reset collision flag when blocks separate
        if (collisionHappened && !block1.collidesWith(block2)) {
            collisionHappened = false;
        }

        // Hide collision text after 2 seconds
        if (showCollisionText && collisionTimer.getElapsedTime().asSeconds() > 2.0f) {
            showCollisionText = false;
        }

        //Bounce off walls
        block1.bounceOffWalls(800);
        block2.bounceOffWalls(800);

        // drawing
        window.clear(sf::Color(240, 240, 240));

        // draw around line
        sf::RectangleShape ground(sf::Vector2f(800,2));
        ground.setPosition(0, 350);
        ground.setFillColor(sf::Color(150, 150, 150));
        window.draw(ground);

        //Draw Blocks
        block1.draw(window);
        block2.draw(window);

        // Draw text info
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::Black);

        // Block 1 info
        std::stringstream ss1;
        ss1 << "Block 1 (Blue)\n"
            << "Position: " << std::fixed << std::setprecision(1) << block1.x << "\n"
            << "Velocity: " << block1.vx << " px/s\n"
            << "Mass: " << block1.mass << " kg";
        text.setString(ss1.str());
        text.setPosition(10, 10);
        window.draw(text);

        // Block 2 info
        std::stringstream ss2;
        ss2 << "Block 2 (Red)\n"
            << "Position: " << std::fixed << std::setprecision(1) << block2.x << "\n"
            << "Velocity: " << block2.vx << " px/s\n"
            << "Mass: " << block2.mass << " kg";
        text.setString(ss2.str());
        text.setPosition(10, 120);
        window.draw(text);

        // Collision alert
        if (showCollisionText) {
            sf::Text collisionText;
            collisionText.setFont(font);
            collisionText.setCharacterSize(30);
            collisionText.setFillColor(sf::Color::Red);
            collisionText.setString("COLLISION!");
            collisionText.setPosition(300, 50);
            window.draw(collisionText);
        }

        if (showEnergyText) {

            // Total system energy
            sf::Text energyText;
            energyText.setFont(font);
            energyText.setCharacterSize(18);
            energyText.setFillColor(sf::Color::Black);
            
            // KE = 1/2mv^2 for each block
            float block1_energy = .5 * block1.mass * block1.vx * block1.vx;
            float block2_energy = .5 * block2.mass * block2.vx * block2.vx;
            float totalEnergy = block1_energy + block2_energy;

            std::stringstream ssEnergy;
            ssEnergy << "Total System Energy: " << std::fixed << std::setprecision(2) << totalEnergy << " J";
            energyText.setString(ssEnergy.str());
            energyText.setString(ssEnergy.str());

            sf::FloatRect textbounds = energyText.getLocalBounds();
            energyText.setPosition(800 - textbounds.width - 10, 10);
            window.draw(energyText);

            // Instructions
            text.setCharacterSize(16);
            text.setString("Press R to Reset");
            text.setPosition(10, 550);
            window.draw(text);
            
            window.display();
        }

    }

    return 0;
}
