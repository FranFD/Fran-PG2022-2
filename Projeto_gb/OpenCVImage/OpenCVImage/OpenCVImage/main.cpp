#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>

#define DEBUG_DETECT (0)

using namespace cv;
using namespace std::chrono_literals;

/*
    Armazena o estado do mouse
    posicao, dragging, se esta pressionado ou nao, o id do sticker selecionado pelo mouse, o botao que foi clicado ou que esta pressionado
*/
struct MouseState {
    int x, y;                   // posicao do mouse
    bool dragging = false;      // o mouse está arrastando(pressionado e movendo)
    bool mouseDown = false;     // o mouse está pressionado
    bool mouseDownFrame = false; // o mouse está pressionado no frame atual
    bool mouseUpFrame = false;  //o mouse foi solto no frame atual
    int stickerIdx = -1;        //id do sticker que está sendo posicionado na imagem(sendo arrastado pelo mouse)
    int stickerInImageIdx = -1; // id do sticker já posicionado na imagem que o mouse está arrastando(dragging)
    int hotButton = 0;      // id do botao que o mouse esta pressionado(down)
    int clickedButton = 0; // id do botao clicado no frame atual
};

// Representa um sticker - Armazena dados para que ele seja renderizado na linha superior da tela, e também em cima da imagem principal
struct Sticker {
    Mat image;      // imagem do sticker
    Mat icon;       // imagem do sticker no tamanho de um icone
    Mat iconMask;   // mascara da camada alfa do icone
    Mat mask;       // mascara da camada alfa da imagem
    Rect container; // posicao do icone do sticker na tela
};

struct App;

// Representa um botao
struct Button {
    int id;                         // identificador do botao
    const char* text;               // texto do botao
    Vec3b color;                    // cor de fundo do botao
    Rect rect;                      // Posicao e tamanho do botao na tela
    void (*onClick)(App* app);      // callback que é invocada quando o botão é pressionado
};

// Representa a imagem sendo renderizada na tela, pode ser carregada de um arquivo ou uma foto tirada da webcam
struct SelectedImage {
    Mat image;          // imagem original(carregada ou uma foto tirada da webcam)
    Mat edited;         // imagem editada com filtros
    Mat preview;        // Preview da imagem original em tamaho menor
    Rect container;     // Posicao e tamanho da imagem na tela
};

// Representa as propriedades de um sticker posicionado a imagem
struct StickerInImage {
    int stickerIdx;     // id do sticker na imagem
    int x, y;           // Posicao do sticker na imagem
};

struct DynamicSticker {
    Mat icon, iconMask;
    Mat left, leftMask, right, rightMask;
    Rect iconRect;
    Rect leftEye, rightEye;
    void (*DetectObjects)(App* app, Mat& image);
    bool active;
};

// Representa um filtro que pode ser aplicado na imagem
struct Filter {
    Mat preview;        // preview da imagem com o filtro aplicado
    Rect container;     // posicao e tamanho do preview do filtro na tela
    void (*FilterFunc)(Mat& src, Mat& dst); // callback, invocada quando o filtro é selecionado
};

// Representa os dados da aplicacao, janela, filtros, botoes, stickers, mouse, etc.
struct App {
    const char* mainWindow; // id da janela da aplicacao

    Mat canvas; // canvas onde tudo é desenhado

    std::vector<Sticker> stickers; // array de stickers

    SelectedImage displayImage;                 // imagem que sera desenhada na tela
    std::vector<StickerInImage> stickersInImage; //array de stickers que estao posicionados na imagem

    std::vector<Button> buttons; // array de botoes

    std::vector<Filter> filters;    // array de filtros
    int appliedFilterIdx = -1;  // indice do filtro aplicado

    MouseState mouseState;

    VideoCapture videoCapture; // usado para captura de video
    DynamicSticker dynamicSticker;
    CascadeClassifier faceDetect;
    CascadeClassifier eyeDetect;
};

// teste se dois rects estao um sob o outro
// React a = imagem, React b = Sticker
bool testRect(Rect a, Rect b) {
    if (a.x > b.x + b.width) return false;
    if (a.x + a.width < b.x) return false;
    if (a.y > b.y + b.height) return false;
    if (a.y + a.height < b.y) return false;
    return true;
}

// Inicia o uso da webcam para tirar a foto ou realiza a captura da foto
void TakePicture(App* app) {
    if (!app->videoCapture.isOpened()) {
        // caso nao esteja capturando video, comeca a capturar e altera o texto do botao
        app->videoCapture = VideoCapture(0);
        app->buttons[1].text = "Capturar";
    }
    else {
        // caso ja esteja capturando, realiza a captura do frame atual da webcam e atualiza os filtros e a imagem principal
        Mat frame;
        app->buttons[1].text = "Tirar Foto";
        app->videoCapture.read(frame);
        app->videoCapture.release();
        app->displayImage.container = Rect(0, 130, frame.cols, frame.rows);
        // faz a copia do frame capturado para a imagem principal
        frame.copyTo(app->displayImage.image);
        frame.copyTo(app->displayImage.edited);
        // atualiza o preview da imagem principal
        resize(app->displayImage.image, app->displayImage.preview, Size(64, 64));

        // apaga os previews de todos os filtros, eles serao recalculados na funcao que desenha os filtros
        for (int i = 0; i < app->filters.size(); ++i) {
            app->filters[i].preview.release();
        }
    }
}


// Faz a detecção da face, depois do olho esquerdo, e depois do direito
void DetectEyes(App* app, Mat& image) {
    Mat grayFrame;
    cvtColor(image, grayFrame, COLOR_BGR2GRAY);

    std::vector<Rect> objects;
    app->faceDetect.detectMultiScale(grayFrame, objects, 1.3f, 5);

    app->dynamicSticker.leftEye.x = -1;
    app->dynamicSticker.rightEye.x = -1;

    if (objects.size() > 0) {
#if DEBUG_DETECT > 0
        rectangle(image, objects[i], Scalar(255, 0, 0, 255), 2);
#endif
        Mat grayFace = grayFrame(objects[0]);

        std::vector<Rect> eyeObjects;
        Rect eyeLeft, eyeRight;
        app->eyeDetect.detectMultiScale(grayFace, eyeObjects, 1.3f, 5);
#if DEBUG_DETECT > 0
        for (int j = 0; j < eyeObjects.size(); ++j) {
            rectangle(image, Rect(eyeObjects[j].x + objects[i].x, eyeObjects[j].y + objects[i].y, eyeObjects[j].width, eyeObjects[j].height), Scalar(0, 255, 255), 2);
        }
#endif

        if (eyeObjects.size() > 1) {
            eyeLeft = eyeObjects[0].x < eyeObjects[1].x ? eyeObjects[0] : eyeObjects[1];
            eyeRight = eyeObjects[0].x < eyeObjects[1].x ? eyeObjects[1] : eyeObjects[0];
        }

        if (eyeObjects.size() == 1) {
            eyeLeft = eyeObjects[0];
        }

        if (eyeObjects.size() > 0) {
            Mat& leftIcon = app->dynamicSticker.left;
            float leftX = (eyeLeft.x + eyeLeft.width / 2 - leftIcon.cols / 2);
            float leftY = eyeLeft.y - leftIcon.rows*0.5f;
            app->dynamicSticker.leftEye = Rect(objects[0].x + leftX, objects[0].y + leftY, leftIcon.cols, leftIcon.rows);
        }

        if (eyeObjects.size() > 1) {
            Mat& rightIcon = app->dynamicSticker.right;
            float rightX = (eyeRight.x + eyeRight.width / 2 - rightIcon.cols / 2);
            float rightY = eyeRight.y - rightIcon.rows*0.5f;
            app->dynamicSticker.rightEye = Rect(objects[0].x + rightX, objects[0].y + rightY, rightIcon.cols, rightIcon.rows);
        }
    }
}

// Aplica o filtro de gray scale
void GrayScaleFilter(Mat& image, Mat& edited) {
    Mat gray;
    // aplica o filtro
    cvtColor(image, gray, COLOR_BGR2GRAY);

    // cria as tres camadas com camadas de cinza e coloca na imagem editada
    Mat c1 = Mat3b(gray.cols, gray.rows, Vec3b(0, 0, 0));
    Mat c2 = Mat3b(gray.cols, gray.rows, Vec3b(0, 0, 0));
    Mat cs[3] = { gray, gray, gray };
    merge(cs, 3, edited);
}

// Applica o filtro de brilho
void BrightnessFilter(Mat& image, Mat& edited) {
    convertScaleAbs(image, edited, 1, 60);
}

// Applica o filtro de escurecimento
void DarkerFilter(Mat& image, Mat& edited) {
    convertScaleAbs(image, edited, 1, -60);
}

// Applica o filtro sharp
void SharpFilter(Mat& image, Mat& edited) {
    // Cria a matriz kernel
    Mat kernel = (Mat_<double>(3, 3) << -1, -1, -1, -1, 9.5, -1, -1, -1, -1);
    filter2D(image, edited, -1, kernel);
}

// Applica o filtro de sepia
void SepiaFilter(Mat& image, Mat& edited) {
    Mat transformMat = (Mat_<double>(3, 3) << 0.272, 0.534, 0.131, 0.349, 0.686, 0.168, 0.393, 0.769, 0.189);
    transform(image, edited, transformMat);
}

// applica o filtro de lapis
void PencilSketchFilter(Mat& image, Mat& edited) {
    Mat gray, color;
    pencilSketch(image, gray, color);
    edited = color;
}

// applica o filtro de lapis em escala cinza
void GrayPencilSketchFilter(Mat& image, Mat& edited) {
    Mat gray, color;
    pencilSketch(image, gray, color);
    // cria a imagem com as tres camadas em cinza
    Mat array[3] = { gray, gray, gray };
    merge(array, 3, edited);
}

// Aplica o filtro bilateral
void BilateralFilter(Mat& image, Mat& edited) {
    for (int i = 1; i < 31; i = i + 2) {
        bilateralFilter(image, edited, i, i * 2, i / 2);
    }
}

// Pplica Aplica o filtro de detalhamento
void HDRFilter(Mat& image, Mat& edited) {
    detailEnhance(image, edited);
}

// Applica o filtro de inversao
void InvertFilter(Mat& image, Mat& edited) {
    bitwise_not(image, edited);
}

// Essa funcao lida com as entradas de mouse do usuário, detectando cliques, movimentos, arrastos, etc.
void mouseCb(int event, int x, int y, int flags, void* userdata) {
    App* app = (App*)userdata;
    bool justClicked = false;
    bool justReleased = false;

    Point mousePos = Point(x, y);

    if (event == EVENT_LBUTTONDOWN) {
        // salva os dados do mouse
        justClicked = !app->mouseState.mouseDown;
        app->mouseState.mouseDown = true;
        app->mouseState.x = x;
        app->mouseState.y = y;

        // verifica se pressionou algum botao
        for (int i = 0; i < app->buttons.size(); ++i) {
            if (app->buttons[i].rect.contains(mousePos)) {
                app->mouseState.hotButton = app->buttons[i].id; // botao pressionado
            }
        }
    }
    else if (event == EVENT_LBUTTONUP) {
        justReleased = app->mouseState.mouseDown;
        // lógica de arrasto quando o usuário solta o sticker arrastado
        if (app->mouseState.dragging) {
            // Está arrastando um sticker
            if (app->mouseState.stickerIdx > -1) {
                Sticker* sticker = &app->stickers[app->mouseState.stickerIdx];
                Rect rect = Rect(x, y, sticker->image.cols, sticker->image.rows);

                if (testRect(rect, app->displayImage.container)) {
                    // se o sticker foi solto sobre a imagem, adiciona ele na lista de sticker na imagem
                    app->stickersInImage.push_back({
                        app->mouseState.stickerIdx,
                        x, y
                        });
                }
            }
            // Está arrastando um sticker que já foi posto na imagem
            else if (app->mouseState.stickerInImageIdx > -1) {
                StickerInImage* sticker = &app->stickersInImage[app->mouseState.stickerInImageIdx];
                Sticker* _sticker = &app->stickers[sticker->stickerIdx];
                Rect rect = Rect(sticker->x, sticker->y, _sticker->image.cols, _sticker->image.rows);

                if (!testRect(rect, app->displayImage.container)) {
                    // se o sticker foi solto fora da imagem, remove ele da lista
                    app->stickersInImage.erase(app->stickersInImage.begin() + app->mouseState.stickerInImageIdx);
                }
            }
        }

        // Lógica de clique quando o usuário solta o botao que estava pressionado
        if (app->mouseState.hotButton) {
            for (int i = 0; i < app->buttons.size(); ++i) {
                if (app->mouseState.hotButton == app->buttons[i].id && app->buttons[i].rect.contains(mousePos)) {
                    app->mouseState.clickedButton = app->mouseState.hotButton;
                }
            }
            app->mouseState.hotButton = 0;
        }

        // Lógica de clique quando o usuário solta o filtro que estava pressionado
        for (int i = app->filters.size() - 1; i >= 0; --i) {
            if (app->filters[i].container.contains(mousePos)) {
                app->appliedFilterIdx = i;
                app->filters[i].FilterFunc(app->displayImage.image, app->displayImage.edited);
                app->buttons[2].text = "Reset Filtros";
                break;
            }
        }

        // clique no dynamic stick (botão)
        if (app->videoCapture.isOpened() && app->dynamicSticker.iconRect.contains(mousePos)) {
            app->dynamicSticker.active = !app->dynamicSticker.active;
            if (!app->dynamicSticker.active) {
                app->dynamicSticker.leftEye.x = -1;
                app->dynamicSticker.rightEye.x = -1;
            }
        }

        // atualiza dados do mouse
        app->mouseState.dragging = false;
        app->mouseState.mouseDown = false;
        app->mouseState.stickerInImageIdx = -1;
        app->mouseState.stickerIdx = -1;
    }
    else if (event == EVENT_MOUSEMOVE) {
        // mouse em movimento
        app->mouseState.x = x;
        app->mouseState.y = y;
        app->mouseState.dragging = app->mouseState.mouseDown && (app->mouseState.stickerIdx > -1 || app->mouseState.stickerInImageIdx > -1);

        // atualiza posicao do sticker que está sendo arrastado na imagem
        if (app->mouseState.dragging && app->mouseState.stickerInImageIdx > -1) {
            app->stickersInImage[app->mouseState.stickerInImageIdx].x = x;
            app->stickersInImage[app->mouseState.stickerInImageIdx].y = y;
        }
    }

    // usuário acabou de pressionar o mouse
    if (justClicked) {
        Point mousePoint = Point(app->mouseState.x, app->mouseState.y);

        // verifica se algum sticker foi clicado
        for (int i = 0; i < app->stickers.size(); ++i) {
            Sticker* sticker = &app->stickers[i];
            if (sticker->container.contains(mousePoint)) {
                app->mouseState.stickerIdx = i; // atualiza sticker sendo selecionado
            }
        }

        // verifica se algum sticker que esta na imagem foi clicado
        for (int i = 0; i < app->stickersInImage.size(); ++i) {
            StickerInImage* sticker = &app->stickersInImage[i];
            Rect stickerRect = Rect(sticker->x, sticker->y, app->stickers[sticker->stickerIdx].image.cols, app->stickers[sticker->stickerIdx].image.rows);
            if (stickerRect.contains(mousePoint)) {
                app->mouseState.stickerInImageIdx = i; // atualiza sticker na imagem que está sendo selecionado
            }
        }
    }
}

// Desenha os stickers em icone na linha superior da tela
void DrawStickers(App* app) {
    int stickersX = 220;
    int stickersY = 10;
    for (int i = 0; i < app->stickers.size(); ++i) {
        Sticker& sticker = app->stickers[i];

        int w = sticker.icon.cols;
        int h = sticker.icon.rows;
        Rect stickerRect = Rect(stickersX + (i * 50) + 25 - (w / 2), stickersY + 25 - h / 2, w, h);
        Rect stickerRectArround = Rect(stickersX + i * 50, stickersY, 50, 50);
        Vec3b containerColor = Vec3b(255, 255, 255);
        // se o sticker estiver pressionado, muda a cor do quadrado dele
        if (app->mouseState.mouseDown && app->mouseState.stickerIdx == i) {
            containerColor = Vec3b(150, 155, 255);
        }
        // desenha o sticker e o quadrado
        app->canvas(stickerRectArround) = containerColor;
        rectangle(app->canvas, stickerRectArround, Scalar(0, 0, 255), 2);
        sticker.icon.copyTo(app->canvas(stickerRect), sticker.iconMask);
        sticker.container = stickerRectArround;
    }
    // Muda a cor do botão
    if (app->videoCapture.isOpened()) {
        app->dynamicSticker.iconRect = Rect(220, 70, 50, 50);
        Vec3b containerColor = Vec3b(255, 255, 255);
        Vec3b borderColor = Vec3b(30, 30, 30);

        if (app->mouseState.mouseDown && app->dynamicSticker.iconRect.contains(Point(app->mouseState.x, app->mouseState.y))) {
            containerColor = Vec3b(150, 155, 255);
        }

        if (app->dynamicSticker.active) {
            borderColor = Vec3b(0, 0, 255);
        }

        app->canvas(app->dynamicSticker.iconRect) = containerColor;
        rectangle(app->canvas, app->dynamicSticker.iconRect, borderColor, 2);
        app->dynamicSticker.icon.copyTo(app->canvas(Rect(236, 86, 32, 32)), app->dynamicSticker.iconMask);
    }
}

// desenha os botoes
void DrawButtons(App* app) {
    for (int i = 0; i < app->buttons.size(); ++i) {
        Button* button = &app->buttons[i];
        // a cor da borda do botao é calculada de acordo com a cor do botao
        Scalar borderColor = Scalar(button->color[0] / 2, button->color[1] / 2, button->color[2] / 2);

        // Determina a cor do botao, se estiver sendo pressionado tera a mesma cor da borda
        if (app->mouseState.hotButton == button->id) {
            app->canvas(button->rect) = borderColor;
        }
        else {
            app->canvas(button->rect) = button->color;
        }
        // obtém o tamanho do texto do botao
        Size textSize = getTextSize(button->text, FONT_HERSHEY_PLAIN, 1, 2, nullptr);
        // desenha o texto do botao com o texto centralizado
        putText(
            app->canvas(button->rect),
            button->text,
            Point(button->rect.width / 2 - textSize.width / 2, button->rect.height / 2 - textSize.height / 2),
            FONT_HERSHEY_PLAIN,
            1,
            Scalar(0, 0, 0),
            2,
            8,
            false
        );
        // desenha a borda do botao
        rectangle(app->canvas, button->rect, borderColor, 2, 2);
    }

    // verifica se o botao foi clicado no frame atual e invoca a callback do botao
    if (app->mouseState.clickedButton) {
        for (int i = 0; i < app->buttons.size(); ++i) {
            Button* button = &app->buttons[i];
            if (button->id == app->mouseState.clickedButton) {
                if (button->onClick) {
                    button->onClick(app);
                }
                break;
            }
        }
    }
}

// Desenha os filtros na linha inferior
void DrawFilters(App* app) {
    float startX = 230, y = 640;
    for (int i = 0; i < app->filters.size(); ++i) {
        Filter* filter = &app->filters[i];
        // se o filtro nao foi aplicado no preview da imagem, aplica o filtro no preview
        if (!filter->preview.data) {
            filter->FilterFunc(app->displayImage.preview, filter->preview);
            filter->container = Rect(startX + i * 62, y, 64, 64);
        }
        // desenha o filtro na parte inferior
        filter->preview.copyTo(app->canvas(filter->container));
    }
}

// desenha a imagem principal com os stickers em cima
void DrawImage(App* app) {
    Mat* renderImage = nullptr;
    Rect displayRect;
    Rect windowRect = getWindowImageRect(app->mainWindow);
    Mat videoFrame;

    // captura de video esta aberta
    if (app->videoCapture.isOpened()) {
        app->videoCapture.read(videoFrame); // le o frame atual 
        if (videoFrame.data) { // se tiver algum dado na frame atual seta para ser renderizado
            renderImage = &videoFrame;

            if (app->dynamicSticker.active) {
                app->dynamicSticker.DetectObjects(app, videoFrame);
            }
        }
    }

    if (!renderImage) { // se a captura de video nao estiver ativa, desenha a imagem que está configurada
        renderImage = &app->displayImage.edited;
    }

    // calcula o retangulo onde a imagem vai ser desenhada, no centro da tela
    displayRect.x = windowRect.width / 2 - renderImage->cols / 2;
    displayRect.y = 130;
    displayRect.width = renderImage->cols;
    displayRect.height = renderImage->rows;

    // desenha a imagem na tela
    renderImage->copyTo(app->canvas(displayRect));
    app->displayImage.container = displayRect;

    // desenha os sticker
    for (int i = 0; i < app->stickersInImage.size(); ++i) {
        int stickerIdx = app->stickersInImage[i].stickerIdx;
        Sticker& sticker = app->stickers[stickerIdx];
        // calcula o rect do sticker
        Rect stickerRect = Rect(app->stickersInImage[i].x, app->stickersInImage[i].y, sticker.image.cols, sticker.image.rows);

        Rect windowRect = getWindowImageRect(app->mainWindow);

        // calcula para nao desenhar o sticker fora da tela
        if (stickerRect.x < 0) stickerRect.x = 0;
        if (stickerRect.x + stickerRect.width > windowRect.width) stickerRect.x = windowRect.width - stickerRect.width;
        if (stickerRect.y < 0) stickerRect.y = 0;
        if (stickerRect.y + stickerRect.height > windowRect.height) stickerRect.y = windowRect.height - stickerRect.height;

        // desenha o sticker
        sticker.image.copyTo(app->canvas(stickerRect), sticker.mask);

        // se o sticker estiver selcionado, desenha uma borda vermelha ao redor dele
        if (app->mouseState.dragging && app->mouseState.stickerInImageIdx > -1 && app->mouseState.stickerInImageIdx == i) {
            rectangle(app->canvas, stickerRect, Scalar(0, 0, 255), 2, 2);
        }
    }
    //Desenha "cílios" no olho esquerdo
    Rect left = app->dynamicSticker.leftEye;
    if (left.x > 0) {
        Rect r(left.x + app->displayImage.container.x, left.y + app->displayImage.container.y, left.width, left.height);
        if (r.x > 0 && r.y > 0 && r.x + r.width < app->canvas.cols && r.y + r.height < app->canvas.rows) {
            app->dynamicSticker.left.copyTo(app->canvas(r), app->dynamicSticker.leftMask);
        }
    }
    //Desenha "cilios" no olho direito
    Rect right = app->dynamicSticker.rightEye;
    if (right.x > 0) {
        Rect r(right.x + app->displayImage.container.x, right.y + app->displayImage.container.y, right.width, right.height);
        if (r.x > 0 && r.y > 0 && r.x + r.width < app->canvas.cols && r.y + r.height < app->canvas.rows) {
            app->dynamicSticker.right.copyTo(app->canvas(r), app->dynamicSticker.rightMask);
        }
    }
}

// Essa funcao vai gerar a imagem final que o usuário salvar
void DrawImageFinal(App* app) {
    if (app->videoCapture.isOpened()) return; // nao salva a imagem se estiver gravando
    int left = app->displayImage.container.x;
    int right = app->displayImage.container.x + app->displayImage.container.width;
    int top = app->displayImage.container.y;
    int bottom = app->displayImage.container.y + app->displayImage.container.height;

    Rect leftEyeRect(-1, -1, -1, -1), rightEyeRect(-1, -1, -1, -1);
    if (app->dynamicSticker.leftEye.x > 0) {
        leftEyeRect = Rect(app->dynamicSticker.leftEye.x + app->displayImage.container.x, app->dynamicSticker.leftEye.y + app->displayImage.container.y, app->dynamicSticker.leftEye.width, app->dynamicSticker.leftEye.height);
        if (leftEyeRect.x < left) left = leftEyeRect.x;
        if (leftEyeRect.x + leftEyeRect.width > right) right = leftEyeRect.x + leftEyeRect.width;
        if (leftEyeRect.y < top) top = leftEyeRect.y;
        if (leftEyeRect.y + leftEyeRect.height > bottom) bottom = leftEyeRect.y + leftEyeRect.height;
    }

    if (app->dynamicSticker.rightEye.x > 0) {
        rightEyeRect = Rect(app->dynamicSticker.rightEye.x + app->displayImage.container.x, app->dynamicSticker.rightEye.y + app->displayImage.container.y, app->dynamicSticker.rightEye.width, app->dynamicSticker.rightEye.height);
        if (rightEyeRect.x < left) left = rightEyeRect.x;
        if (rightEyeRect.x + rightEyeRect.width > right) right = rightEyeRect.x + rightEyeRect.width;
        if (rightEyeRect.y < top) top = rightEyeRect.y;
        if (rightEyeRect.y + rightEyeRect.height > bottom) bottom = rightEyeRect.y + rightEyeRect.height;
    }

    // calcula a posicao left, right, bottom e top dos pixels da imagem/stickers(alguns stickers podem nao estar completamente dentro da imagem)
    for (int i = 0; i < app->stickersInImage.size(); ++i) {
        int stickerIdx = app->stickersInImage[i].stickerIdx;
        Sticker& sticker = app->stickers[stickerIdx];
        Rect stickerRect = Rect(app->stickersInImage[i].x, app->stickersInImage[i].y, sticker.image.cols, sticker.image.rows);

        if (app->stickersInImage[i].x < left) left = app->stickersInImage[i].x;
        if (app->stickersInImage[i].x + sticker.image.cols > right) right = app->stickersInImage[i].x + sticker.image.cols;
        if (app->stickersInImage[i].y < top) top = app->stickersInImage[i].y;
        if (app->stickersInImage[i].y + sticker.image.rows > bottom) bottom = app->stickersInImage[i].y + sticker.image.rows;
    }

    // canvas onde a imagem final será renderizada, com as dimensoes previamente calculadas
    Mat canvas = Mat3b(bottom - top, right - left, Vec3b(255, 255, 255));

    // rect para centraliar a imagem no canvas
    Rect displayRect;
    displayRect.x = canvas.cols / 2 - app->displayImage.edited.cols / 2;
    displayRect.y = canvas.rows / 2 - app->displayImage.edited.rows / 2;
    displayRect.width = app->displayImage.edited.cols;
    displayRect.height = app->displayImage.edited.rows;

    // desenha a imagem no canvas
    app->displayImage.edited.copyTo(canvas(displayRect));
       
    // desenha os stickers na imagem
    for (int i = 0; i < app->stickersInImage.size(); ++i) {
        int stickerIdx = app->stickersInImage[i].stickerIdx;
        Sticker& sticker = app->stickers[stickerIdx];
        // rect do sticker, subtraido das posicoes calculadas anteriormente
        Rect stickerRect = Rect(app->stickersInImage[i].x - left, app->stickersInImage[i].y - top, sticker.image.cols, sticker.image.rows);
        sticker.image.copyTo(canvas(stickerRect), sticker.mask);
    }

    if (leftEyeRect.x > 0) {
        Rect stickerRect = Rect(leftEyeRect.x - left, leftEyeRect.y - top, leftEyeRect.width, leftEyeRect.height);
        app->dynamicSticker.left.copyTo(canvas(stickerRect), app->dynamicSticker.leftMask);
    }

    if (rightEyeRect.x > 0) {
        Rect stickerRect = Rect(rightEyeRect.x - left, rightEyeRect.y - top, rightEyeRect.width, rightEyeRect.height);
        app->dynamicSticker.right.copyTo(canvas(stickerRect), app->dynamicSticker.rightMask);
    }

    // salva a imagem
    imwrite("output.png", canvas);
}

// desenha o sticker sendo arrastado
void DrawDrag(App* app) {
    if (app->mouseState.dragging && app->mouseState.stickerIdx > -1) {
        Sticker& sticker = app->stickers[app->mouseState.stickerIdx];
        Rect rect = Rect(app->mouseState.x, app->mouseState.y, sticker.image.cols, sticker.image.rows);
        Rect windowRect = getWindowImageRect(app->mainWindow);

        // calculado para nao renderizar o sticker fora da tela
        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.width > windowRect.width) rect.x = windowRect.width - rect.width;
        if (rect.y < 0) rect.y = 0;
        if (rect.y + rect.height > windowRect.height) rect.y = windowRect.height - rect.height;

        sticker.image.copyTo(app->canvas(rect), sticker.mask);
    }
}

// remove os filtros da imagem
void ClearImage(App* app) {
    app->displayImage.image.copyTo(app->displayImage.edited); // copia a imagem original para a imagem editada
    app->appliedFilterIdx = -1;
    app->buttons[2].text = "Filtros";
}

// mostra a tela de carregamento inicial
void ShowLoading(App* app) {
    Mat canvas = Mat3b(800, 1200, Vec3b(100, 200, 180));

    const char* text = "Loading...";

    Size textSize = getTextSize(text, FONT_HERSHEY_PLAIN, 1, 2, nullptr);
    putText(
        canvas,
        text,
        Point(600 - textSize.width / 2, 400 - textSize.height / 2),
        FONT_HERSHEY_PLAIN,
        1,
        Scalar(0, 0, 0),
        2,
        8,
        false
    );

    imshow(app->mainWindow, canvas);
}

// logica para carregar a imagem inicial do programa
void LoadAppImage(App* app) {
    std::string imageName;
    
    bool valid = false;
    Mat image;

    do {
        std::cout << "Insira o caminho da imagem: ";
        std::cin >> imageName;
        std::cout << "Carregando imagem " << imageName << std::endl;
        image = cv::imread(imageName, IMREAD_COLOR); // lê a imagem atual

        if (!image.data) {
            std::cout << "Imagem nao encontrada\n";
            valid = false;
        }
        else if (image.cols < 32 || image.rows < 32) {
            std::cout << "Imagem invalida, deve ter as dimensoes de pelo menos 128x128 pixels\n";
            valid = false;
        }
        else {
            valid = true;
        }
    } while (!valid);


    image.copyTo(app->displayImage.image); // copia a imagem para o app

    // Ajusta as dimensoes da imagem para caber no canvas, sem alterar as proporcoes
    if (image.cols > 800 || image.rows > 500) {
        int width, height;
        float ratio = (float)image.cols / image.rows;
        if (image.cols > image.rows) {
            height = 800 / ratio;
            width = 800;
        }
        else {
            width = 500 * ratio;
            height = 500;
        }
        // redimensiona
        resize(image, app->displayImage.image, Size(width, height));
    }
    // salva a imagem editada copia da original
    app->displayImage.image.copyTo(app->displayImage.edited);
    // cria o preview da imagem
    resize(app->displayImage.image, app->displayImage.preview, Size(64, 64));
}

//Remove a transparência das imagens e passa para fundo branco
void Mask(Mat& input, Mat& image, Mat& imageMask, Mat& icon, Mat& iconMask) {
    resize(input, icon, Size(32, 32));

    std::vector<Mat> rgbLayer;
    split(input, rgbLayer);
    Mat cs[3] = { rgbLayer[0],rgbLayer[1],rgbLayer[2] };
    merge(cs, 3, input);
    imageMask = rgbLayer[3];
    input.copyTo(image);

    std::vector<Mat> iconRgbLayer;
    split(icon, iconRgbLayer);
    Mat cs2[3] = { iconRgbLayer[0],iconRgbLayer[1],iconRgbLayer[2] };
    merge(cs2, 3, icon);
    iconMask = iconRgbLayer[3];
}

int main(int argc, char* argv[]) {
    // lista de stickers
    const char* StickersNames[] = {
        "Stickers/Caveman.png",
        "Stickers/GotWolf.png",
        "Stickers/HarryPotter.png",
        "Stickers/OnePiece.png",
        "Stickers/Pikachu.png"
    };
    App app;

    LoadAppImage(&app);

    app.mainWindow = "OpenCV Filters"; // id/titulo da janela
    
    // carrega os stickers
    app.stickers.resize(5);
    for (int i = 0; i < 5; ++i) {
        Mat image = cv::imread(StickersNames[i], IMREAD_UNCHANGED);
        if (!image.data) throw "Failed to load image";
        Mask(image, app.stickers[i].image, app.stickers[i].mask, app.stickers[i].icon, app.stickers[i].iconMask);
    }

    // cria os botoes
    app.buttons.push_back({
        1,
        "Stickers",
        Vec3b(200, 100, 50),
        Rect(10, 10, 200, 50),
        nullptr
    });

    app.buttons.push_back({
        2,
        "Tirar Foto",
        Vec3b(200, 100, 50),
        Rect(10, 70, 200, 50),
        TakePicture
    });

    app.buttons.push_back({
        3,
        "Filtros",
        Vec3b(200, 100, 50),
        Rect(10, 640, 200, 50),
        ClearImage
    });

    app.buttons.push_back({
        4,
        "Salvar",
        Vec3b(200, 100, 50),
        Rect(10, 700, 200, 50),
        DrawImageFinal
        });

    // cria os filtros e configura as callbacks
    app.filters.resize(10);
    app.filters[0].FilterFunc = GrayScaleFilter;
    app.filters[1].FilterFunc = BrightnessFilter;
    app.filters[2].FilterFunc = DarkerFilter;
    app.filters[3].FilterFunc = SharpFilter;
    app.filters[4].FilterFunc = SepiaFilter;
    app.filters[5].FilterFunc = PencilSketchFilter;
    app.filters[6].FilterFunc = GrayPencilSketchFilter;
    app.filters[7].FilterFunc = HDRFilter;
    app.filters[8].FilterFunc = InvertFilter;
    app.filters[9].FilterFunc = BilateralFilter;

    // Dynamic Sticker icon
    Mat left = cv::imread("Stickers/RightEye.png", IMREAD_UNCHANGED);
    Mat right = cv::imread("Stickers/LeftEye.png", IMREAD_UNCHANGED);
    Mask(right, app.dynamicSticker.right, app.dynamicSticker.rightMask, app.dynamicSticker.icon, app.dynamicSticker.iconMask);
    Mask(left, app.dynamicSticker.left, app.dynamicSticker.leftMask, app.dynamicSticker.icon, app.dynamicSticker.iconMask);
    app.dynamicSticker.active = 0;
    app.dynamicSticker.DetectObjects = DetectEyes;
    app.eyeDetect.load("haarcascade_eye.xml");
    app.faceDetect.load("haarcascade_frontalface_default.xml");

    // cria a janela e seta a callback do mouse
    namedWindow(app.mainWindow);
    setMouseCallback(app.mainWindow, mouseCb, &app);

    // mostra a tela de carregamento
    ShowLoading(&app);
    waitKey(1);

    // loop do app, roda enquanto a janela nao for fechada
    while (getWindowProperty(app.mainWindow, WindowPropertyFlags::WND_PROP_VISIBLE) == 1) {
        app.canvas = Mat3b(800, 1200, Vec3b(100, 100, 180)); // recria o canvas para renderizar o frame atual
        DrawStickers(&app);
        DrawButtons(&app);
        DrawFilters(&app);
        DrawImage(&app);
        DrawDrag(&app);

        // Reseta as informacoes especificas do frame atual, antes de processar eventos do mouse
        app.mouseState.mouseDownFrame = false;
        app.mouseState.mouseUpFrame = false;
        app.mouseState.clickedButton = 0;

        // desenha a imagem na tela
        imshow(app.mainWindow, app.canvas);
        waitKey(1);
    }

    return 0;
}