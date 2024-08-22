#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <vector>

//Input Output
#include <iostream>
#include <iomanip>

//Time for FPS
#include <chrono>

//QT
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFontDatabase>
#include <QImage>
#include <QTransform>

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

QMap<int, bool> keyStates;

#define TILES_X 32
#define TILES_Y 18
#define RATIO_V 25.0
#define RATIO_H 25.0
#define MARGIN 0.05

QRectF scale(QRectF rect){
    return QRectF(rect.left()*RATIO_H, rect.top()*RATIO_V, rect.width()*RATIO_H, rect.height()*RATIO_V);
}


struct Entity{
    QRectF box;

    bool solid=true;
    bool visible=true;
    bool movable=false;

    bool* stateSolid=nullptr;
    bool* stateVisible=nullptr;
    bool* statePressed=nullptr;
    bool* stateMovable=nullptr;

    void update(QRectF box){
        if(stateSolid!=nullptr){
            solid=!(*stateSolid);
        }
        if(stateVisible!=nullptr){
            visible=!(*stateVisible);
        }
        if(statePressed!=nullptr){
            if(this->box.intersects(box))
                *statePressed=true;
        }
        if(stateMovable!=nullptr){
            movable=stateMovable;
        }
    }
};

struct Tile:public Entity{

    Tile(QRectF box){
        this->box=box;
    }
};

struct Door:public Entity{

    Door(QRectF box){
        this->box=box;
    }

};

struct Button:public Entity{

    Button(QRectF box){
        this->box=box;
        solid=false;
    }

};

struct Platform:public Entity{

    std::vector<QPointF> points;
    float speed;

    Platform(QRectF box){
        this->box=box;
    }

    QRectF motionState(){
        QRectF out;
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        std::vector<float> distances;
        float totalDistance=0;
        for(int i=0;i<points.size();i++){
            QPointF dif=points.at(i);
            if(i!=points.size()-1)
                dif-=points.at(i+1);
            else
                dif-=points.at(0);
            distances.push_back(sqrt(dif.x()*dif.x()+dif.y()*dif.y()));
            totalDistance+=distances.at(i);
        }

        int timeStamp=ms.count();
        int timeSeed=((int)(timeStamp%(int)(1000/speed)*speed)%1000);

        float raw=(float)(timeSeed/1000.0);
        float distance=raw*totalDistance;
        for(int i=0;i<distances.size();i++){
            if(distance>distances.at(i)){
                distance-=distances.at(i);
            }
            else{
                QPointF pos=points.at(i);
                if(i!=points.size()-1)
                    pos+=(points.at(i+1)-points.at(i))*(distance/distances.at(i));
                else
                    pos+=(points.at(0)-points.at(i))*(distance/distances.at(i));
                out=QRectF(pos, QSizeF(box.width(), box.height()));
                return out;
            }
        }
    }
};

struct Sign{
    int ID;
    bool* state;
};

struct SignalList{
    std::vector<Sign> list;

    Sign sign(int ID){
        for(int i=0;i<list.size();i++)
            if(list.at(i).ID==ID)
                return list.at(i);
        Sign s;
        s.ID=ID;
        s.state=new bool;
        list.push_back(s);
        return list.at(list.size()-1);
    }
};


struct World{
    std::vector<Tile> tiles;
    std::vector<Door> doors;
    std::vector<Button> buttons;
    std::vector<Platform> platforms;
    SignalList signalList;

    int a;
    int b;
    int c;

    void json(const QString &filePath) {
        QFile file(filePath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QString jsonString = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());



        QJsonArray jsonArrayBase = jsonDoc.array();

        for(const auto& jsonValue : jsonArrayBase){

            QJsonObject jsonObj=jsonValue.toObject();

            if(jsonObj["type"].toString()==QString("tile")){
                jsonObj=jsonObj["data"].toObject();
                Tile tile(QRectF(jsonObj["x"].toDouble(), jsonObj["y"].toDouble(), jsonObj["dx"].toDouble(), jsonObj["dy"].toDouble()));
                if(jsonObj["stateSolid"].toInt())
                    tile.stateSolid=signalList.sign(jsonObj["stateSolid"].toInt()).state;
                if(jsonObj["stateVisible"].toInt())
                    tile.stateVisible=signalList.sign(jsonObj["stateVisible"].toInt()).state;
                if(jsonObj["statePressed"].toInt())
                    tile.statePressed=signalList.sign(jsonObj["statePressed"].toInt()).state;
                if(jsonObj["stateMovable"].toInt())
                    tile.stateMovable=signalList.sign(jsonObj["stateMovable"].toInt()).state;
                if(jsonObj["solid"].toInt())
                    tile.solid=signalList.sign(jsonObj["solid"].toBool()).state;
                if(jsonObj["visible"].toInt())
                    tile.visible=signalList.sign(jsonObj["visible"].toBool()).state;
                if(jsonObj["movable"].toInt())
                    tile.movable=signalList.sign(jsonObj["movable"].toBool()).state;
                tiles.push_back(tile);
                std::cout<<"#"<<std::endl;
            }
            if(jsonObj["type"].toString()==QString("door")){
                jsonObj=jsonObj["data"].toObject();
                Door door(QRectF(jsonObj["x"].toDouble(), jsonObj["y"].toDouble(), jsonObj["dx"].toDouble(), jsonObj["dy"].toDouble()));
                if(jsonObj["stateSolid"].toInt())
                    door.stateSolid=signalList.sign(jsonObj["stateSolid"].toInt()).state;
                if(jsonObj["stateVisible"].toInt())
                    door.stateVisible=signalList.sign(jsonObj["stateVisible"].toInt()).state;
                if(jsonObj["statePressed"].toInt())
                    door.statePressed=signalList.sign(jsonObj["statePressed"].toInt()).state;
                if(jsonObj["stateMovable"].toInt())
                    door.stateMovable=signalList.sign(jsonObj["stateMovable"].toInt()).state;
                if(jsonObj["solid"].toInt())
                    door.solid=signalList.sign(jsonObj["solid"].toBool()).state;
                if(jsonObj["visible"].toInt())
                    door.visible=signalList.sign(jsonObj["visible"].toBool()).state;
                if(jsonObj["movable"].toInt())
                    door.movable=signalList.sign(jsonObj["movable"].toBool()).state;
                doors.push_back(door);
                std::cout<<"#"<<std::endl;
            }
            if(jsonObj["type"].toString()==QString("button")){
                jsonObj=jsonObj["data"].toObject();
                Button button(QRectF(jsonObj["x"].toDouble(), jsonObj["y"].toDouble(), jsonObj["dx"].toDouble(), jsonObj["dy"].toDouble()));
                if(jsonObj["stateSolid"].toInt())
                    button.stateSolid=signalList.sign(jsonObj["stateSolid"].toInt()).state;
                if(jsonObj["stateVisible"].toInt())
                    button.stateVisible=signalList.sign(jsonObj["stateVisible"].toInt()).state;
                if(jsonObj["statePressed"].toInt())
                    button.statePressed=signalList.sign(jsonObj["statePressed"].toInt()).state;
                if(jsonObj["stateMovable"].toInt())
                    button.stateMovable=signalList.sign(jsonObj["stateMovable"].toInt()).state;
                if(jsonObj["solid"].toInt())
                    button.solid=signalList.sign(jsonObj["solid"].toBool()).state;
                if(jsonObj["visible"].toInt())
                    button.visible=signalList.sign(jsonObj["visible"].toBool()).state;
                if(jsonObj["movable"].toInt())
                    button.movable=signalList.sign(jsonObj["movable"].toBool()).state;
                buttons.push_back(button);
                std::cout<<"#"<<std::endl;
            }
            if(jsonObj["type"].toString()==QString("platform")){
                jsonObj=jsonObj["data"].toObject();
                Platform platform(QRectF(0, 0, jsonObj["dx"].toDouble(), jsonObj["dy"].toDouble()));
                QJsonArray jsonArray = jsonObj["points"].toArray();
                for(const auto& jsonValue2 : jsonArray){
                    QJsonObject jsonObj2=jsonValue2.toObject();
                    platform.points.push_back(QPoint(jsonObj2["x"].toInt(), jsonObj2["y"].toInt()));
                }
                platform.speed=jsonObj["speed"].toDouble();
                if(jsonObj["stateSolid"].toInt())
                    platform.stateSolid=signalList.sign(jsonObj["stateSolid"].toInt()).state;
                if(jsonObj["stateVisible"].toInt())
                    platform.stateVisible=signalList.sign(jsonObj["stateVisible"].toInt()).state;
                if(jsonObj["statePressed"].toInt())
                    platform.statePressed=signalList.sign(jsonObj["statePressed"].toInt()).state;
                if(jsonObj["stateMovable"].toInt())
                    platform.stateMovable=signalList.sign(jsonObj["stateMovable"].toInt()).state;
                if(jsonObj["solid"].toInt())
                    platform.solid=signalList.sign(jsonObj["solid"].toBool()).state;
                if(jsonObj["visible"].toInt())
                    platform.visible=signalList.sign(jsonObj["visible"].toBool()).state;
                if(jsonObj["movable"].toInt())
                    platform.movable=signalList.sign(jsonObj["movable"].toBool()).state;
                platforms.push_back(platform);
                std::cout<<"#"<<std::endl;
            }
        }
    }


    World(QString filenameMap, QString filenameButton, QString filenameDoor){
        QPixmap pixmap1(filenameMap);
        QPixmap pixmap2(filenameDoor);
        QPixmap pixmap3(filenameButton);

        QImage image1 = pixmap1.toImage();
        QImage image2 = pixmap2.toImage();
        QImage image3 = pixmap3.toImage();

        for (int j=0; j < image1.height(); j++) {
            for (int i=0; i < image1.width(); i++) {
                QColor color = image1.pixelColor(i, j);
                if(color!=QColor(255, 255, 255)){
                    Tile tile(QRectF(i, j, 1, 1));
                    tiles.push_back(tile);
                }
            }
        }

        for (int j=0; j < image2.height(); j++) {
            for (int i=0; i < image2.width(); i++) {
                QColor color = image2.pixelColor(i, j);
                if(color!=QColor(255, 255, 255)){
                    Door door(QRectF(i, j, 1, 1));
                    door.stateSolid=signalList.sign(color.red()+256*color.green()+256*256*color.blue()).state;
                    door.stateVisible=signalList.sign(color.red()+256*color.green()+256*256*color.blue()).state;
                    doors.push_back(door);
                }
            }
        }

        for (int j=0; j < image3.height(); j++) {
            for (int i=0; i < image3.width(); i++) {
                QColor color = image3.pixelColor(i, j);
                if(color!=QColor(255, 255, 255)){
                    Button button(QRectF(i, j, 1, 1));
                    button.statePressed=signalList.sign(color.red()+256*color.green()+256*256*color.blue()).state;
                    button.stateVisible=signalList.sign(color.red()+256*color.green()+256*256*color.blue()).state;
                    button.solid=false;
                    buttons.push_back(button);
                }
            }
        }

        json("Entities.json");
    }



    void print(QPainter &painter, QRectF mapRect){
        QTransform tr;
        tr.rotate(90);

        QImage imageTile;
        imageTile.load("tileBase.png");
        QImage imageDoor;
        imageDoor.load("door.png");
        QImage imageButton;
        imageButton.load("button.png");
        QImage imagePlatform;
        imagePlatform.load("tileBase.png");
        QImage imageBlack;
        imageBlack.load("black.png");

        for(int i=0;i<tiles.size();i++)
            if(tiles.at(i).visible)
                painter.drawImage(scale(QRectF(tiles.at(i).box.x()-mapRect.x()+TILES_X/2, tiles.at(i).box.y()-mapRect.y()+TILES_Y/2, tiles.at(i).box.width(), tiles.at(i).box.height())), imageTile);

        for(int i=0;i<doors.size();i++)
            if(doors.at(i).visible)
                painter.drawImage(scale(QRectF(doors.at(i).box.x()-mapRect.x()+TILES_X/2, doors.at(i).box.y()-mapRect.y()+TILES_Y/2, doors.at(i).box.width(), doors.at(i).box.height())), imageDoor);

        for(int i=0;i<buttons.size();i++)
            if(buttons.at(i).visible)
                painter.drawImage(scale(QRectF(buttons.at(i).box.x()-mapRect.x()+TILES_X/2, buttons.at(i).box.y()-mapRect.y()+TILES_Y/2, buttons.at(i).box.width(), buttons.at(i).box.height())), imageButton);




        for(int i=0;i<platforms.size();i++)
            if(platforms.at(i).visible)
                painter.drawImage(scale(
                                      QRectF(
                                          platforms.at(i).motionState().x()-mapRect.x()+TILES_X/2,
                                          platforms.at(i).motionState().y()-mapRect.y()+TILES_Y/2,
                                          platforms.at(i).motionState().width(),
                                          platforms.at(i).motionState().height())), imageTile);

    }
};


struct Player{
    float x;
    float y;
    float vx;
    float vy;

    QRectF outBox(){
        return QRectF(x, y, 1.0, 1.0);
    }

    QRectF box(){
        return QRectF(x+0.1, y+0.2, 0.8, 0.8);
    }

    QRectF underBox(){
        return QRectF(x+0.1, y+1.0, 0.8, 0.01);
    }

    QRectF mapRect(){
        float outX=x;
        float outY=y;
        while(outX>TILES_X-0.5)
            outX-=TILES_X;
        while(outY>TILES_Y-0.5)
            outY-=TILES_Y;
        return QRectF(x, y, TILES_X, TILES_Y);
    }

    void tick(World &world){

        if(vx>-0.35 && keyStates[Qt::Key_Left])
            vx-=0.01;
        if(vx<0.35 && keyStates[Qt::Key_Right])
            vx+=0.01;
        vx=vx*0.9;

        if(keyStates[Qt::Key_Up]){
            for(int i=0;i<world.tiles.size();i++)
                if(world.tiles.at(i).box.intersects(underBox())&&vy>=0)
                    vy=-0.18;
        }
        if(vy<0.4)
            vy+=0.003;

        checkBoxH(world);
        checkBoxV(world);

        y+=vy;
        if(checkBoxV(world))
            vy=0;
        x+=vx;
        checkBoxH(world);

        for(int i=0;i<world.tiles.size();i++){
            world.tiles.at(i).update(box());
        }
        for(int i=0;i<world.buttons.size();i++){
            world.buttons.at(i).update(box());
        }
        for(int i=0;i<world.doors.size();i++){
            world.doors.at(i).update(box());
        }

    }

    bool colisionV(QRectF rect){
        if(rect.intersects(box())){
            if(rect.center().y()>box().center().y()){
                y-=box().bottom()-rect.top()+MARGIN;
            }
            else{
                y+=rect.bottom()-box().top()+MARGIN;
            }
            return true;
        }
        return false;
    }

    bool colisionH(QRectF rect){
        if(rect.intersects(box())){
            if(rect.center().x()>box().center().x()){
                x-=box().right()-rect.left()+MARGIN;
            }
            else{
                x+=rect.right()-box().left()+MARGIN;
            }
            return true;
        }
        return false;
    }

    bool checkBoxV(World world){
        for(int i=0;i<world.tiles.size();i++){
            if(world.tiles.at(i).solid)
                if(colisionV(world.tiles.at(i).box))
                    return true;
        }
        for(int i=0;i<world.doors.size();i++){
            if(world.doors.at(i).solid)
                if(colisionV(world.doors.at(i).box))
                    return true;
        }
        for(int i=0;i<world.buttons.size();i++){
            if(world.buttons.at(i).solid)
                if(colisionV(world.buttons.at(i).box))
                    return true;
        }
        for(int i=0;i<world.platforms.size();i++){
            if(world.platforms.at(i).solid)
                if(colisionV(world.platforms.at(i).motionState()))
                    return true;
        }
        return false;
    }

    bool checkBoxH(World world){
        for(int i=0;i<world.tiles.size();i++){
            if(world.tiles.at(i).solid)
                if(colisionH(world.tiles.at(i).box))
                    return true;
        }
        for(int i=0;i<world.doors.size();i++){
            if(world.doors.at(i).solid)
                if(colisionH(world.doors.at(i).box))
                    return true;
        }
        for(int i=0;i<world.buttons.size();i++){
            if(world.buttons.at(i).solid)
                if(colisionH(world.buttons.at(i).box))
                    return true;
        }
        for(int i=0;i<world.platforms.size();i++){
            if(world.platforms.at(i).solid)
                if(colisionH(world.platforms.at(i).motionState()))
                    return true;
        }
        return false;
    }
};



class CustomLabel : public QLabel{

public:

    Player player;
    World world=World("map.bmp", "button.bmp", "door.bmp");


    CustomLabel(){
        player.x=8;
        player.y=8;
        for(int i=0;i<1000;i++)
            keyStates[i]=false;
    }

private:




protected:

    void paintEvent(QPaintEvent* event) override {
        this->setFocus();
        QLabel::paintEvent(event);

        static long long lastTime=0;

        auto currentTime = std::chrono::system_clock::now();
        auto cTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

        if(cTime-lastTime>4){
            lastTime=cTime;
            player.tick(world);
        }


        // Set pen
        QPainter painter(this);
        QPen pen(Qt::black, 2);
        painter.setPen(pen);

        // Paint the base
        painter.fillRect(rect().left(), rect().top(), rect().right(), rect().bottom(), QColor(0x0c,0x29,0x2a));
        //painter.drawRect(rect().left(), rect().top(), rect().right(), rect().bottom());


        world.print(painter, player.mapRect());


        QImage imagePlayer;
        if(player.vx>=0)
            imagePlayer.load("playerR.png");
        else
            imagePlayer.load("playerL.png");
        painter.drawImage(scale(QRectF(TILES_X/2, TILES_Y/2, player.outBox().width(), player.outBox().height())), imagePlayer);


        update();
    }

    void mousePressEvent(QMouseEvent* event) override {
        int x = event->pos().x();
        int y = event->pos().y();

        update();
    }



    void mouseReleaseEvent(QMouseEvent* event) override {
        int x = event->pos().x();
        int y = event->pos().y();

        update();
    }



    void mouseMoveEvent(QMouseEvent* event) override {
        int x = event->pos().x();
        int y = event->pos().y();

        update();
    }

    void wheelEvent(QWheelEvent *event) override {
        if (true) {
            //scroll += event->angleDelta().y()/5;
        }
        else {
        }
    }

    void keyPressEvent(QKeyEvent *event) override {
        //event->text().at(0).unicode()
        if(event->key()==Qt::Key_Left)
            keyStates[Qt::Key_Left]=true;
        if(event->key()==Qt::Key_Right)
            keyStates[Qt::Key_Right]=true;
        if(event->key()==Qt::Key_Up)
            keyStates[Qt::Key_Up]=true;
    }

    void keyReleaseEvent(QKeyEvent *event) override {
        //event->text().at(0).unicode()
        if(event->key()==Qt::Key_Left)
            keyStates[Qt::Key_Left]=false;
        if(event->key()==Qt::Key_Right)
            keyStates[Qt::Key_Right]=false;
        if(event->key()==Qt::Key_Up)
            keyStates[Qt::Key_Up]=false;
    }



};



#endif // CUSTOMLABEL_H
