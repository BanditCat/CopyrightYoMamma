import Graphics.Rendering.OpenGL
import Graphics.UI.GLUT
import Data.IORef
import System



data Camera = Camera {position, direction, delta :: (GLfloat, GLfloat, GLfloat)}



main = do
  (progname,_) <- getArgsAndInitialize
  initialDisplayMode $= [DoubleBuffered]
  initialWindowSize $= Size 1024 1024
  createWindow "Hello World"
  reshapeCallback $= Just reshape
  angle <- newIORef (0.0::GLfloat)
  delta <- newIORef (0.1::GLfloat)
  position <- newIORef (0.0::GLfloat, 0.0)
  keyboardMouseCallback $= Just (keyboardMouse delta position)
  idleCallback $= Just (idle angle delta)
  displayCallback $= (display angle position)
  mainLoop


reshape s@(Size w h) = do 
  viewport $= (Position 0 0, s)
 
keyboardAct a p (Char ' ') Down = do
  a' <- get a
  a $= -a'
keyboardAct a p (Char '+') Down = do
  a' <- get a
  a $= 2*a'
keyboardAct a p (Char '-') Down = do
  a' <- get a
  a $= a'/2
keyboardAct a p (Char '\ESC') Down = do
  exitWith ExitSuccess
keyboardAct a p (SpecialKey KeyLeft) Down = do
  (x,y) <- get p
  p $= (x-0.1,y)
keyboardAct a p (SpecialKey KeyRight) Down = do
  (x,y) <- get p
  p $= (x+0.1,y)
keyboardAct a p(SpecialKey KeyUp) Down = do
  (x,y) <- get p
  p $= (x,y+0.1)
keyboardAct a p (SpecialKey KeyDown) Down = do
  (x,y) <- get p
  p $= (x,y-0.1)
keyboardAct _ _ _ _ = return ()
 
keyboardMouse angle pos key state modifiers position = do
  keyboardAct angle pos key state

cubeFrame w = renderPrimitive Lines $ vertify3
  [ ( w,-w, w), ( w, w, w),  ( w, w, w), (-w, w, w),
    (-w, w, w), (-w,-w, w),  (-w,-w, w), ( w,-w, w),
    ( w,-w, w), ( w,-w,-w),  ( w, w, w), ( w, w,-w),
    (-w, w, w), (-w, w,-w),  (-w,-w, w), (-w,-w,-w),
    ( w,-w,-w), ( w, w,-w),  ( w, w,-w), (-w, w,-w),
    (-w, w,-w), (-w,-w,-w),  (-w,-w,-w), ( w,-w,-w) ]
  
vertify3 :: [(GLfloat,GLfloat,GLfloat)] -> IO ()
vertify3 verts = sequence_ $ map (\(a,b,c) -> vertex $ Vertex3 a b c) verts 
  
display angle position = do 
  clear [ColorBuffer,DepthBuffer] --added DepthBuffer to list of things to be cleared
  loadIdentity
  (x,y) <- get position
  translate $ Vector3 x y 0
  preservingMatrix $ do 
    a <- get angle
    rotate a $ Vector3 0 0.1 (1::GLfloat) --change y-component of axis of rotation to show off cube corners
    scale 0.7 0.7 (0.7::GLfloat)
    mapM_ (\(x,y,z) -> preservingMatrix $ do
      color $ Color3 ((x+1.0)/2.0) ((y+1.0)/2.0) ((z+1.0)/2.0)
      translate $ Vector3 x y z
      cube (0.1::GLfloat)
      color $ Color3 (0.0::GLfloat) (0.0::GLfloat) (0.0::GLfloat) --set outline color to black
      cubeFrame (0.1::GLfloat) --draw the outline
      ) $ points 7
  swapBuffers
points :: Int -> [(GLfloat,GLfloat,GLfloat)]
points n' = let n = fromIntegral n' in
            map (\k -> let t = 2*pi*k/n in (sin(t),cos(t),0.0))  [1..n]
            
idle angle delta = do
  a <- get angle
  d <- get delta
  angle $=! (a+d)
  postRedisplay Nothing            
  
cube w = do 
  renderPrimitive Quads $ do
    vertex $ Vertex3 w w w
    vertex $ Vertex3 w w (-w)
    vertex $ Vertex3 w (-w) (-w)
    vertex $ Vertex3 w (-w) w
    vertex $ Vertex3 w w w
    vertex $ Vertex3 w w (-w)
    vertex $ Vertex3 (-w) w (-w)
    vertex $ Vertex3 (-w) w w
    vertex $ Vertex3 w w w
    vertex $ Vertex3 w (-w) w
    vertex $ Vertex3 (-w) (-w) w
    vertex $ Vertex3 (-w) w w
    vertex $ Vertex3 (-w) w w
    vertex $ Vertex3 (-w) w (-w)
    vertex $ Vertex3 (-w) (-w) (-w)
    vertex $ Vertex3 (-w) (-w) w
    vertex $ Vertex3 w (-w) w
    vertex $ Vertex3 w (-w) (-w)
    vertex $ Vertex3 (-w) (-w) (-w)
    vertex $ Vertex3 (-w) (-w) w
    vertex $ Vertex3 w w (-w)
    vertex $ Vertex3 w (-w) (-w)
    vertex $ Vertex3 (-w) (-w) (-w)
    vertex $ Vertex3 (-w) w (-w)
    
