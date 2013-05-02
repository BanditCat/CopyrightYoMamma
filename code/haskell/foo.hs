-- TODO: make instaces of Show, make randomgen functions take size parameter, subExpression -> subexpression
-- then removeRandomSubinstruction, then removeRandomSubprogram, then write breed using all of them

module Main where
import qualified Crypto.Random.AESCtr as AES                                    
import qualified Data.ByteString.Char8 as B
import Data.Bits
import Data.Set
import Data.Char
import Data.Maybe
import Data.Array
import System.Random

rnd :: AES.AESRNG
rnd = rnd' $ AES.make $ B.pack "nonce5 Smoke Cannabis everyday123451231243513516131356afbkabkafbaskhkasbasdkbasgaghgdkbasdk"
  where rnd' (Left _) = error "error"
        rnd' (Right x) = x

anyNatural :: AES.AESRNG -> (Int, AES.AESRNG)
anyNatural r = if b then ((an' + 1) * 2 + v,r''') else (v,r'')
  where (b, r') = (random r :: (Bool, AES.AESRNG))
        (v, r'') = randomR (0,1) r'
        (an', r''') = anyNatural r''

anyNaturals :: AES.AESRNG -> [Int]
anyNaturals r = an : (anyNaturals r')
  where (an,r') = anyNatural r

primes = 2 : primes'
  where primes' = sieve [3,5..] 9 primes'
        sieve (x:xs) q ps@ ~(p:t)
          | x < q     = x : sieve xs q ps
          | otherwise =     sieve (xs `minus` [q, q+2*p..]) (head t^2) t
        minus (x:xs) (y:ys) = case (compare x y) of
          LT -> x : minus  xs  (y:ys)
          EQ ->     minus  xs     ys
          GT ->     minus (x:xs)  ys
        minus  xs     _     = xs



data Instruction = Sequence Instruction Instruction | 
                   Insert Expression | 
                   Delete Expression | 
                   Move Expression | 
                   Loop Expression Expression (Int -> Int -> Bool) String Instruction | 
                   Branch Expression Expression (Int -> Int -> Bool) String Instruction Instruction | 
                   GraftInstruction
data Expression = StackValue Expression | 
                  StackSize | 
                  InputSize | 
                  InputWord Expression | 
                  Value Int | 
                  ExpressionArithmetic Expression Expression (Int -> Int -> Int) String | 
                  ExpressionBranch Expression Expression (Int -> Int -> Bool) String Expression Expression |
                  GraftExpression

--allExpressions :: Integer -> [Expression]
--allExpressions n
--  | n == 0 = [StackSize, InputSize]
--  | 


instance Eq Expression where
  (StackValue a) == (StackValue b) = (a == b)
  StackSize == StackSize = True
  InputSize == InputSize = True
  (InputWord a) == (InputWord b) = (a == b)
  (Value a) == (Value b) = (a == b)
  (ExpressionArithmetic a b _ s) == (ExpressionArithmetic c d _ t) = (a == c) && (b == d) && (s == t)
  (ExpressionBranch a b _ s c d) == (ExpressionBranch e f _ t g h) = (a == e) && (b == f) && (s == t) && (c == g) && (d == h)
  GraftExpression == GraftExpression = True
  _ == _ = False

instance Ord Expression where
  compare (ExpressionBranch a1 b1 _ s1 c1 d1) (ExpressionBranch a2 b2 _ s2 c2 d2)
    | cm1 /= EQ = cm1
    | cm2 /= EQ = cm2
    | cm3 /= EQ = cm3
    | cm4 /= EQ = cm4
    | otherwise = cm5
    where cm1 = compare a1 a2
          cm2 = compare b1 b2
          cm3 = compare s1 s2
          cm4 = compare c1 c2
          cm5 = compare d1 d2
  compare (ExpressionBranch _ _ _ _ _ _) _ = GT
  compare _ (ExpressionBranch _ _ _ _ _ _) = LT
  compare (ExpressionArithmetic a1 b1 _ s1) (ExpressionArithmetic a2 b2 _ s2)
    | cm1 /= EQ = cm1
    | cm2 /= EQ = cm2
    | otherwise = cm3
    where cm1 = compare a1 a2
          cm2 = compare b1 b2
          cm3 = compare s1 s2
  compare (ExpressionArithmetic _ _ _ _) _ = GT
  compare _ (ExpressionArithmetic _ _ _ _) = LT
  compare (StackValue a1) (StackValue a2) = compare a1 a2
  compare (StackValue _) _ = GT
  compare _ (StackValue _) = LT
  compare (InputWord a1) (InputWord a2) = compare a1 a2
  compare (InputWord _) _ = GT
  compare _ (InputWord _) = LT
  compare (Value a1) (Value a2) = compare a1 a2
  compare (Value _) _ = GT
  compare _ (Value _) = LT
  compare StackSize StackSize = EQ
  compare StackSize _ = GT
  compare _ StackSize = LT
  compare InputSize InputSize = EQ

instance Eq Instruction where
  (Sequence a b) == (Sequence c d) = (a == c) && (b == d)
  (Insert a) == (Insert b) = (a == b)
  (Delete a) == (Delete b) = (a == b)
  (Move a) == (Move b) = (a == b)
  (Loop a b _ s c) == (Loop d e _ t f) = (a == d) && (b == e) && (s == t) && (c == f)
  (Branch a b _ s c d) == (Branch e f _ t g h) = (a == e) && (b == f) && (s == t) && (c == g) && (d == h)
  GraftInstruction == GraftInstruction = True
  _ == _ = False

instance Ord Instruction where
  compare (Branch a1 b1 _ s1 c1 d1) (Branch a2 b2 _ s2 c2 d2)
    | cm1 /= EQ = cm1
    | cm2 /= EQ = cm2
    | cm3 /= EQ = cm3
    | cm4 /= EQ = cm4
    | otherwise = cm5
    where cm1 = compare a1 a2
          cm2 = compare b1 b2
          cm3 = compare s1 s2
          cm4 = compare c1 c2
          cm5 = compare d1 d2
  compare (Branch _ _ _ _ _ _) _ = GT
  compare _ (Branch _ _ _ _ _ _) = LT
  compare (Loop a1 b1 _ s1 c1) (Loop a2 b2 _ s2 c2)
    | cm1 /= EQ = cm1
    | cm2 /= EQ = cm2
    | cm3 /= EQ = cm3
    | otherwise = cm4
    where cm1 = compare a1 a2
          cm2 = compare b1 b2
          cm3 = compare s1 s2
          cm4 = compare c1 c2
  compare (Loop _ _ _ _ _) _ = GT
  compare _ (Loop _ _ _ _ _) = LT
  compare (Sequence a1 b1) (Sequence a2 b2)
    | cm1 /= EQ = cm1
    | otherwise = cm2
    where cm1 = compare a1 a2
          cm2 = compare b1 b2
  compare (Sequence _ _) _ = GT
  compare _ (Sequence _ _) = LT
  compare (Insert a1) (Insert a2) = compare a1 a2
  compare (Insert _) _ = GT
  compare _ (Insert _) = LT
  compare (Delete a1) (Delete a2) = compare a1 a2
  compare (Delete _) _ = GT
  compare _ (Delete _) = LT
  compare (Move a1) (Move a2) = compare a1 a2


instructionSize :: Instruction -> Int
instructionSize (Sequence i1 i2) = 1 + (instructionSize i1) + (instructionSize i2)
instructionSize (Insert _) = 1
instructionSize (Delete _) = 1
instructionSize (Move _) = 1
instructionSize (Loop _ _ _ _ i) = 1 + (instructionSize i)
instructionSize (Branch _ _ _ _ i1 i2) = 1 + (instructionSize i1) + (instructionSize i2)

expressionSize :: Expression -> Int
expressionSize (StackValue e) = 1 + (expressionSize e)
expressionSize StackSize = 1
expressionSize InputSize = 1
expressionSize (InputWord e) = 1 + (expressionSize e)
expressionSize (Value _) = 1
expressionSize (ExpressionArithmetic e1 e2 _ _) = 1 + (expressionSize e1) + (expressionSize e2)
expressionSize (ExpressionBranch e1 e2 _ _ e3 e4) = 1 + (expressionSize e1) + (expressionSize e2)
                                                       + (expressionSize e3) + (expressionSize e4)

programSize :: Instruction -> Int
programSize (Sequence i1 i2) = 1 + (programSize i1) + (programSize i2)
programSize (Insert e) = 1 + (expressionSize e)
programSize (Delete e) = 1 + (expressionSize e)
programSize (Move e) = 1 + (expressionSize e)
programSize (Loop e1 e2 _ _ i) = 1 + (expressionSize e1) + (expressionSize e2) + (programSize i)
programSize (Branch e1 e2 _ _ i1 i2) = 1 + (expressionSize e1) + (expressionSize e2) + (programSize i1) + (programSize i2)

expressionBranchCount :: Expression -> Int
expressionBranchCount (StackValue e) = 1 + (expressionBranchCount e)
expressionBranchCount (InputWord e) = 1 + (expressionBranchCount e)
expressionBranchCount (ExpressionArithmetic e1 e2 _ _) = 1 + (expressionBranchCount e1) + (expressionBranchCount e2)
expressionBranchCount (ExpressionBranch e1 e2 _ _ e3 e4) = 1 + (expressionBranchCount e1) + (expressionBranchCount e2)
                                                       + (expressionBranchCount e3) + (expressionBranchCount e4)
expressionBranchCount _ = 0

instructionBranchCount :: Instruction -> Int
instructionBranchCount (Sequence i1 i2) = 1 + (instructionBranchCount i1) + (instructionBranchCount i2)
instructionBranchCount (Loop e1 e2 _ _ i) = 1 + (instructionBranchCount i)
instructionBranchCount (Branch e1 e2 _ _ i1 i2) = 1 + (instructionBranchCount i1) + (instructionBranchCount i2)
instructionBranchCount _ = 0

totalBranchCount :: Instruction -> Int
totalBranchCount i = tbc' (Left i)
  where tbc' :: (Either Instruction Expression) -> Int
        tbc' (Left (Sequence i1 i2)) = 2 + (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Left (Insert e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Delete e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Move e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Loop e1 e2 _ _ i)) = 3 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i)
        tbc' (Left (Branch e1 e2 _ _ i1 i2)) = 4 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Right (StackValue e)) = 1 + (tbc' $ Right e)
        tbc' (Right (InputWord e)) = 1 + (tbc' $ Right e)
        tbc' (Right (ExpressionArithmetic e1 e2 _ _)) = 2 + (tbc' $ Right e1) + (tbc' $ Right e2)
        tbc' (Right (ExpressionBranch e1 e2 _ _ e3 e4)) = 4 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Right e3) + (tbc' $ Right e4)
        tbc' (Right _) = 0
        
totalInstructionBranchCount :: Instruction -> Int
totalInstructionBranchCount (Sequence i1 i2) = 2 + (totalInstructionBranchCount i1) + (totalInstructionBranchCount i2)
totalInstructionBranchCount (Loop _ _ _ _ i) = 1 + (totalInstructionBranchCount i)
totalInstructionBranchCount (Branch _ _ _ _ i1 i2) = 2 + (totalInstructionBranchCount i1) + (totalInstructionBranchCount i2)
totalInstructionBranchCount _ = 0

  
  
  where tbc' :: (Either Instruction Expression) -> Int
        tbc' (Left (Sequence i1 i2)) = 2 + (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Left (Insert e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Delete e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Move e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Loop e1 e2 _ _ i)) = 3 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i)
        tbc' (Left (Branch e1 e2 _ _ i1 i2)) = 4 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Right (StackValue e)) = 1 + (tbc' $ Right e)
        tbc' (Right (InputWord e)) = 1 + (tbc' $ Right e)
        tbc' (Right (ExpressionArithmetic e1 e2 _ _)) = 2 + (tbc' $ Right e1) + (tbc' $ Right e2)
        tbc' (Right (ExpressionBranch e1 e2 _ _ e3 e4)) = 4 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Right e3) + (tbc' $ Right e4)
        tbc' (Right _) = 0

totalExpressionBranchCount :: Instruction -> Int
totalExpressionBranchCount i = tbc' (Left i)
  where tbc' :: (Either Instruction Expression) -> Int
        tbc' (Left (Sequence i1 i2)) = (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Left (Insert e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Delete e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Move e)) = 1 + (tbc' $ Right e)
        tbc' (Left (Loop e1 e2 _ _ i)) = 2 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i)
        tbc' (Left (Branch e1 e2 _ _ i1 i2)) = 2 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Left i1) + (tbc' $ Left i2)
        tbc' (Right (StackValue e)) = 1 + (tbc' $ Right e)
        tbc' (Right (InputWord e)) = 1 + (tbc' $ Right e)
        tbc' (Right (ExpressionArithmetic e1 e2 _ _)) = 2 + (tbc' $ Right e1) + (tbc' $ Right e2)
        tbc' (Right (ExpressionBranch e1 e2 _ _ e3 e4)) = 4 + (tbc' $ Right e1) + (tbc' $ Right e2) + (tbc' $ Right e3) + (tbc' $ Right e4)
        tbc' (Right _) = 0


expressionToString :: Expression -> String
expressionToString (StackValue v) = "[" ++ (expressionToString v) ++ "]"
expressionToString StackSize = "STACK_SIZE"
expressionToString InputSize = "INPUT_SIZE"
expressionToString (InputWord v) = "<" ++ (expressionToString v) ++ ">"
expressionToString (Value v) = show v
expressionToString (ExpressionArithmetic io ao _ s) = "(" ++ (expressionToString io) ++ s ++ (expressionToString ao) ++ ")"
expressionToString (ExpressionBranch a b t tname tv fv) = "((" ++ (expressionToString a) ++ tname ++ (expressionToString b) ++ ")?(" ++ (expressionToString tv) ++ "):(" ++ (expressionToString fv) ++ "))"
expressionToString GraftExpression = "grafte"


instructionToString :: Instruction -> String
instructionToString (GraftInstruction) = "grafti"
instructionToString (Sequence n m) = (instructionToString n) ++ "\n" ++ (instructionToString m)
instructionToString (Insert i) = "Push " ++ (expressionToString i)
instructionToString (Delete d) = "Delete " ++ (expressionToString d)
instructionToString (Move d) = "Bury " ++ (expressionToString d)
instructionToString (Loop a b _ tname p) = "while(" ++ (expressionToString a) ++ tname ++ (expressionToString b) ++ "){\n" ++ (instructionToString p) ++ "\n}"
instructionToString (Branch a b _ tname tv fv) = "If(" ++ (expressionToString a) ++ tname ++ (expressionToString b) ++ "){\n" ++ (instructionToString tv) ++ "\n}else{\n" ++ (instructionToString fv) ++ "\n}"

stackToString :: (Show x) => [x] -> String
stackToString l = "stack{\n" ++ sts' l 0 ++ "}\n"
 where sts' [] _ = ""
       sts' (h:t) n = "  " ++ (show n) ++ ": " ++ (show h) ++ "\n" ++ (sts' t (n + 1))

programStateToString :: ProgramState -> String
programStateToString (Stop t ss s) = "Stopped. ticks: " ++ (show t) ++ " stack size: " ++ (show ss) ++ "\n" ++ (stackToString s)
programStateToString (Program t ss s i p) = "ticks: " ++ (show t) ++ " stack size: " ++ (show ss) ++ "\n" ++ (stackToString s) ++ "\n" ++ instructionToString p ++ "\n"

getIndex :: Maybe Int -> [Int] -> Maybe Int
getIndex Nothing _ = Nothing
getIndex _ [] = Nothing
getIndex (Just 0) (h:_) = Just h
getIndex (Just n) (h:t) = getIndex (Just (n - 1)) t

getInput :: Maybe Int -> (Array Int Int) -> Maybe Int
getInput Nothing _ = Nothing
getInput (Just ii) i
  | ii > (snd (bounds i)) = Nothing
  | ii < (fst (bounds i)) = Nothing
  | otherwise = (Just (i!ii))


without :: Maybe Int -> [Int] -> Maybe [Int]
without Nothing _ = Nothing
without _ [] = Nothing
without (Just 0) (h:t) = Just t
without (Just x) (h:t) = if (isNothing t') then Nothing else (Just (h:(fromJust t')))
  where t' = without (Just (x - 1)) t

selectRandomlyFromList :: [a] -> AES.AESRNG -> ((a,[a]), AES.AESRNG)
selectRandomlyFromList l rng = (srfl' nv l, rng')
  where (nv, rng') = randomR (1, fromIntegral $ length l) rng :: (Int, AES.AESRNG)
        srfl' 1 (h:t) = (h, t)
        srfl' n (h:t) = (h', h:t')
          where (h', t') = srfl' (n - 1) t


buryFront :: Maybe Int -> [Int] -> Maybe [Int ]
buryFront Nothing _ = Nothing
buryFront _ [] = Nothing
buryFront (Just x) (h:t) = bf' x h t
  where bf' 0 e l = Just (e:l)
        bf' _ _ [] = Nothing
        bf' n e (h:t)
          | isNothing bf'' = Nothing
          | otherwise = Just (h:(fromJust bf''))
          where bf'' = bf' (n - 1) e t

expressionToVal :: Expression -> [Int] -> Int -> (Array Int Int) -> Maybe (Int, Int)
expressionToVal (StackValue v) s ss i 
  | isNothing mv || isNothing s' = Nothing
  | otherwise = Just (fromJust s', t + v' + 1)
  where mv = expressionToVal v s ss i
        (v', t) = fromJust mv
        s' = getIndex (Just v') s
expressionToVal StackSize s ss i = Just (ss, 0)
expressionToVal InputSize s ss i = Just ((snd (bounds i)) + 1, 0)
expressionToVal (InputWord ii) s ss i
  | isNothing mii || isNothing gi = Nothing
  | otherwise = Just (fromJust gi, 1 + t)
  where gi = getInput (Just eii) i
        mii = (expressionToVal ii s ss i)
        (eii, t) = fromJust mii
expressionToVal (Value n) s ss i = Just (n, 0)
expressionToVal (ExpressionArithmetic v ao f _) s ss i
  | isNothing v' = Nothing
  | isNothing ao' = Nothing
  | otherwise = Just (f (fst $ fromJust v') (fst $ fromJust ao'),
                      (snd $ fromJust v') + (snd $ fromJust ao') + 1)
  where v' = expressionToVal v s ss i
        ao' = expressionToVal ao s ss i
expressionToVal (ExpressionBranch a b t tname tv fv) s ss i
  | isNothing a' = Nothing
  | isNothing b' = Nothing
  | t' && isNothing tv' = Nothing
  | (not t') && isNothing fv' = Nothing
  | otherwise = Just (if t' then (fst $ fromJust tv') else (fst $ fromJust fv'),
                      if t' then (evt + (snd $ fromJust tv')) else (evt + (snd $ fromJust fv')))
  where a' = expressionToVal a s ss i
        b' = expressionToVal b s ss i
        t' = (t (fst $ fromJust a') (fst $ fromJust b'))
        tv' = expressionToVal tv s ss i
        fv' = expressionToVal fv s ss i
        evt = (snd $ fromJust a') + (snd $ fromJust b')


data ProgramState = Program { ticks::Int, stackSize::Int, stack::[Int], inputs::(Array Int Int), program::(Instruction) } | Stop { ticks::Int, stackSize::Int, stack::[Int] }

isDone (Program _ _ _ _ _) = False
isDone (Stop _ _ _) = True
getTicks (Program t _ _ _ _) = t
getTicks (Stop t _ _) = t
getStackSize (Program _ ss _ _ _) = ss
getStackSize (Stop _ ss _) = ss
getStack (Program _ _ s _ _) = s
getStack (Stop _ _ s) = s

tick :: ProgramState -> ProgramState
tick st@(Stop t ss s) = st
tick (Program t ss s i GraftInstruction) = (Stop t ss s)
tick (Program t ss s i (Sequence GraftInstruction y)) = tick (Program t ss s i y)
tick (Program t ss s i (Sequence x y)) = if (isDone ps') then ps' else ps''
  where ps' = tick (Program t ss s i x)
        ps'' = (Program t' ss' s' i (Sequence x' y))
        t' = ticks ps'
        ss' = stackSize ps'
        s' = stack ps'
        x' = program ps'
tick (Program t ss s i (Insert x))
  | isNothing x' = (Stop t ss s)
  | otherwise = (Program (t + (snd $ fromJust x') + 1) (ss + 1) ((fst $ fromJust x'):s) i GraftInstruction)
  where x' = expressionToVal x s ss i
tick (Program t ss s i (Delete x))
  | isNothing x' || isNothing s' = (Stop t ss s)
  | otherwise = (Program (t + (fst $ fromJust x') + (snd $ fromJust x')) (ss - 1) (fromJust s') i GraftInstruction)
  where x' = expressionToVal x s ss i
        s' = without (Just $ fst $ fromJust x') s
tick (Program t ss s i (Move x))
  | isNothing x' || isNothing s' = (Stop t ss s)
  | otherwise = (Program (t + (fst $ fromJust x') + (snd $ fromJust x')) ss (fromJust s') i GraftInstruction)
  where x' = expressionToVal x s ss i
        s' = buryFront (Just $ fst $ fromJust x') s
tick (Program t ss s i l@(Loop a b tst tstname p)) = if ((isNothing a') || (isNothing b')) 
                                                     then (Stop t ss s) 
                                                     else (tick' t ss s i (fst $ fromJust a') (fst $ fromJust b') tst p)
  where a' = expressionToVal a s ss i
        b' = expressionToVal b s ss i
        tick' t ss s i a b tst p
          | (tst a b) = (Program (t + 1 + (snd $ fromJust a') + (snd $ fromJust b')) ss s i (Sequence p l))
          | otherwise = (Program (t + 1 + (snd $ fromJust a') + (snd $ fromJust b')) ss s i GraftInstruction)
tick (Program t ss s i (Branch a b tst name tv fv))
  | isNothing a' || isNothing b' = Stop t ss s
  | otherwise = Program (t + 1 + (snd $ fromJust a') + (snd $ fromJust b')) ss s i (if (tst (fst $ fromJust a') (fst $ fromJust b')) then tv else fv)
  where a' = expressionToVal a s ss i
        b' = expressionToVal b s ss i

run :: Instruction -> (Array Int Int) -> Int -> Int -> ProgramState
run p i maxTicks maxStack = run' (Program 0 0 [] i p) maxTicks maxStack
  where run' ps@(Stop _ _ _) _ _ = ps
        run' ps maxTicks maxStack
          | (getTicks ps) > maxTicks = ps
          | (getStackSize ps) > maxStack = ps
          | otherwise = run' (tick ps) maxTicks maxStack

trace :: Instruction -> (Array Int Int) -> Int -> Int -> String
trace p i maxTicks maxStack = trc' (Program 0 0 [] i p) maxTicks maxStack
  where trc' ps@(Stop _ _ _) _ _ = programStateToString ps
        trc' ps maxTicks maxStack
          | (getTicks ps) > maxTicks = ""
          | (getStackSize ps) > maxStack = ""
          | otherwise = (programStateToString ps) ++ "\n" ++ (trc' (tick ps) maxTicks maxStack)

randomBranch :: AES.AESRNG -> (((Int -> Int -> Bool), String), AES.AESRNG)
randomBranch rng = (rb' nv, rng')
  where (nv, rng') = randomR (0, 5) rng :: (Int, AES.AESRNG)
        rb' 0 = ((\a b -> a == b), "==")
        rb' 1 = ((\a b -> a /= b), "/=")
        rb' 2 = ((\a b -> a < b), "<")
        rb' 3 = ((\a b -> a <= b), "<=")
        rb' 4 = ((\a b -> a > b), ">")
        rb' 5 = ((\a b -> a >= b), ">=")

randomArithmetic :: AES.AESRNG -> (((Int -> Int -> Int), String), AES.AESRNG)
randomArithmetic r = (ra' nv, rng')
  where (nv, rng') = randomR (0, 4) r :: (Int, AES.AESRNG)
        ra' 0 = ((\a b -> a + b), "+")
        ra' 1 = ((\a b -> a - b), "-")
        ra' 2 = ((\a b -> if (b == 0) then 0 else (a `quot` b)), "/")
        ra' 3 = ((\a b -> if (b == 0) then 0 else (a `rem` b)), "%")
        ra' 4 = ((\a b -> a * b), "*")



randomExpression :: AES.AESRNG -> (Expression, AES.AESRNG)
randomExpression rng = rio' nv rng'
  where rngp = randomR (0, 11) rng :: (Int, AES.AESRNG)
        rng' = snd rngp
        nv = fst rngp
        rio' n r
          | n <= 5 = (Value pv, r')
          where (pv,r') = anyNatural r
        rio' 6 r = (StackSize, r)
        rio' 7 r = (InputSize, r)
        rio' 8 r = (InputWord rio'', r')
          where (rio'', r') = randomExpression r
        rio' 9 r = (StackValue rio'', r')
          where (rio'', r') = randomExpression r
        rio' 10 r = (ExpressionArithmetic rio'' rio''' (fst ra) (snd ra), r''')
          where (rio'', r') = randomExpression r
                (rio''', r'') = randomExpression r'
                (ra, r''') = randomArithmetic r''
        rio' 11 r = (ExpressionBranch rio'' rio''' (fst rbo) (snd rbo) rio'''' rio''''', r''''')
          where (rbo, r') = randomBranch r
                (rio'', r'') = randomExpression r'
                (rio''', r''') = randomExpression r''
                (rio'''', r'''') = randomExpression r'''
                (rio''''', r''''') = randomExpression r''''

randomInstruction :: AES.AESRNG -> (Instruction, AES.AESRNG)
randomInstruction rng = ri' nv rng'
  where (nv, rng') = randomR (0, 5) rng :: (Int, AES.AESRNG)
        ri' 0 r = (Sequence ri'' ri''', r''')
          where (nv', r') = randomR (1, 5) r :: (Int, AES.AESRNG)
                (ri'', r'') = ri' nv' r'
                (ri''', r''') = randomInstruction r''
        ri' 1 r = (Insert rio, r')
          where (rio, r') = randomExpression r
        ri' 2 r = (Delete rio, r')
          where (rio, r') = randomExpression r
        ri' 3 r = (Move rio, r')
          where (rio, r') = randomExpression r
        ri' 4 r = (Loop rio rio' (fst rb) (snd rb) ri'', r'''')
          where (rio, r') = randomExpression r
                (rio', r'') = randomExpression r'
                (ri'', r''') = randomInstruction r''
                (rb, r'''') = randomBranch r'''
        ri' 5 r = (Branch rio rio' (fst rb) (snd rb) ri'' ri''', r''''')
          where (rio, r') = randomExpression r
                (rio', r'') = randomExpression r'
                (ri'', r''') = randomInstruction r''
                (ri''', r'''') = randomInstruction r'''
                (rb, r''''') = randomBranch r''''


removeRandomSubinstruction :: Instruction -> AES.AESRNG -> ((Instruction, Maybe Instruction), AES.AESRNG)
removeRandomSubinstruction (Sequence i1 i2) r
  | (nv == 0) && wh = ((Sequence GraftInstruction i2, Just i1), r'')
  | nv == 0 = ((Sequence i1 GraftInstruction, Just i2), r'')
  | nv <= ibc1 = ((se, Just $ Sequence (fromJust rest) i2), r''')
  | otherwise = ((se, Just $ Sequence i1 (fromJust rest)), r''')
  where (nv, r') = randomR (0, ibc1 + ibc2) r :: (Int, AES.AESRNG)
        (wh, r'') = random r :: (Bool, AES.AESRNG)
        ibc1 = instructionBranchCount i1
        ibc2 = instructionBranchCount i2
        ((se, rest), r''') = removeRandomSubinstruction (if (nv <= ibc1) then i1 else i2) r''
removeRandomSubinstruction (Loop e1 e2 t s i) r
  | nv == 0 = ((Loop e1 e2 t s GraftInstruction, Just i), r')
  | otherwise = ((se, Just $ Loop e1 e2 t s (fromJust rest)), r'')
  where (nv, r') = randomR (0, ibc) r :: (Int, AES.AESRNG)
        ibc = instructionBranchCount i
        ((se, rest), r'') = removeRandomSubinstruction i r'
removeRandomSubinstruction (Branch e1 e2 t s i1 i2) r
  | (nv == 0) && wh = ((Branch e1 e2 t s GraftInstruction i2, Just i1), r'')
  | nv == 0 = ((Branch e1 e2 t s i1 GraftInstruction, Just i2), r'')
  | nv <= ibc1 = ((se, Just $ Branch e1 e2 t s (fromJust rest) i2), r''')
  | otherwise = ((se, Just $ Branch e1 e2 t s i1 (fromJust rest)), r''')
  where (nv, r') = randomR (0, ibc1 + ibc2) r :: (Int, AES.AESRNG)
        (wh, r'') = random r :: (Bool, AES.AESRNG)
        ibc1 = instructionBranchCount i1
        ibc2 = instructionBranchCount i2
        ((se, rest), r''') = removeRandomSubinstruction (if (nv <= ibc1) then i1 else i2) r''
removeRandomSubinstruction e r = ((e, Nothing), r)

removeRandomSubinstructionFromList :: [Instruction] -> AES.AESRNG -> ((Instruction, [Instruction]), AES.AESRNG)
removeRandomSubinstructionFromList l rng = rfl' nv l rng'
  where (nv, rng') = randomR (1, fromIntegral $ length l) rng :: (Int, AES.AESRNG)
        rfl' 1 (h:t) r
          | isNothing h' = ((se, t), r')
          | otherwise = ((se, (fromJust h'):t), r')
          where ((se, h'), r') = removeRandomSubinstruction h r
        rfl' n (h:t) r = ((se, (h:t')), r')
          where ((se, t'), r') = rfl' (n - 1) t r

shrinkInstruction :: Instruction -> AES.AESRNG -> (Instruction, AES.AESRNG)
shrinkInstruction i r
  | b = (i, r')
  | (isNothing rest) = (i, r'')
  | otherwise = (i', r''')
  where (b, r') = random r :: (Bool, AES.AESRNG)
        ((_, rest), r'') = removeRandomSubinstruction i r'
        (i', r''') = shrinkInstruction (fromJust rest) r''

graftInstruction :: Instruction -> Instruction -> (Bool, Instruction)
graftInstruction GraftInstruction i = (True, i)
graftInstruction (Sequence i1 i2) i = (v1 || v2, Sequence i1' i2') 
  where (v1, i1') = graftInstruction i1 i
        (v2, i2') = graftInstruction i2 i
graftInstruction (Loop e1 e2 t s si) i = (v, Loop e1 e2 t s si')
  where (v, si') = graftInstruction si i
graftInstruction (Branch e1 e2 t s i1 i2) i = (v1 || v2, Branch e1 e2 t s i1' i2') 
  where (v1, i1') = graftInstruction i1 i
        (v2, i2') = graftInstruction i2 i
graftInstruction i _ = (False, i)

graftProgram :: Instruction -> (Either Instruction Expression) -> Instruction
graftProgram i ie = gi (gp' (Left i) ie)
  where gi (Left i) = i
        ge (Right e) = e
        gp' :: (Either Instruction Expression) -> (Either Instruction Expression) -> (Either Instruction Expression)
        gp' (Left (Sequence i1 i2)) ie = Left $ Sequence (gi $ gp' (Left i1) ie) (gi $ gp' (Left i2) ie)
        gp' (Left (Insert e)) ie = Left $ Insert (ge $ gp' (Right e) ie)
        gp' (Left (Delete e)) ie = Left $ Delete (ge $ gp' (Right e) ie)
        gp' (Left (Move e)) ie = Left $ Move (ge $ gp' (Right e) ie)
        gp' (Left (Loop e1 e2 t s i)) ie = Left $ Loop (ge $ gp' (Right e1) ie) (ge $ gp' (Right e2) ie) t s (gi $ gp' (Left i) ie)
        gp' (Left (Branch e1 e2 t s i1 i2)) ie = Left $ Branch (ge $ gp' (Right e1) ie) (ge $ gp' (Right e2) ie) t s (gi $ gp' (Left i1) ie) (gi $ gp' (Left i2) ie)
        gp' (Left GraftInstruction) (Left i) = Left i
        gp' (Right (StackValue e)) ie = Right $ StackValue (ge $ gp' (Right e) ie)
        gp' (Right (InputWord e)) ie = Right $ InputWord (ge $ gp' (Right e) ie)
        gp' (Right (ExpressionArithmetic e1 e2 f s)) ie = Right $ ExpressionArithmetic (ge $ gp' (Right e1) ie) (ge $ gp' (Right e2) ie) f s
        gp' (Right (ExpressionBranch e1 e2 f s e3 e4)) ie = Right $ ExpressionBranch (ge $ gp' (Right e1) ie) (ge $ gp' (Right e2) ie) f s (ge $ gp' (Right e3) ie) (ge $ gp' (Right e4) ie)
        gp' (Right GraftExpression) (Right e) = Right e
        gp' (Right e) _ = Right e




{-
breedInstruction :: [Instruction] -> AES.AESRNG -> (Instruction, AES.AESRNG)
breedInstruction l rng = (withExpressions ans nexp, rng''')
  where (ans, rng'') = shrinkInstruction e rng'
        lexp = concatMap allExpressions l
        ansexp = allExpressions ans
        (e, rng') = bi' l rng
        (nexp, rng''') = gb lexp (length ansexp) rng''
        bi' l r
          | (null l') || (not gr) = (se, r')
          | otherwise = (ge, r'')
          where ((se, l'), r') = removeRandomSubinstructionFromList l r
                (ben, r'') = bi' l' r'
                (gr, ge) = graftInstruction se ben
        gb _ 0 r = ([], r)
        gb l n r = (ne:rest, r'')
          where (ne, r') = breedExpression l r
                (rest, r'') = gb l (n - 1) r'
-}
breedInstruction :: [Instruction] -> AES.AESRNG -> (Instruction, AES.AESRNG)                       
breedInstruction [i] r = (i, r)
breedInstruction l r  
  | il bottom = (graftProgram top (Left bi'), ri'''')
  | otherwise = (graftProgram top (Right be'), re'''')
  where ((el, l'), r') = selectRandomlyFromList l r
        (el', r'') = breedInstruction l' r'
        ((top, bottom), r''') = removeRandomCompleteSubprogram el' r''
        il (Left _) = True
        il _ = False
        ((_, bi'), ri'''') = removeRandomCompleteSubinstruction el r'''
        ((_, be'), re'''') = removeRandomCompleteSubexpression el r'''
  
-- Takes an expression and returns it with a peice removed.  It returns a triple of the piece removed, the rest, and the next rng state
removeRandomSubexpression :: Expression -> AES.AESRNG -> ((Expression, Maybe Expression), AES.AESRNG)
removeRandomSubexpression (StackValue e) r 
  | nv == 0 = ((StackValue GraftExpression, Just e), r')
  | otherwise = ((se, Just $ StackValue $ fromJust rest), r'')
  where (nv, r') = randomR (0, expressionBranchCount e) r :: (Int, AES.AESRNG)
        ((se, rest), r'') = removeRandomSubexpression e r'
removeRandomSubexpression (InputWord e) r 
  | nv == 0 = ((InputWord GraftExpression, Just e), r')
  | otherwise = ((se, Just $ InputWord $ fromJust rest), r'')
  where (nv, r') = randomR (0, expressionBranchCount e) r :: (Int, AES.AESRNG)
        ((se, rest), r'') = removeRandomSubexpression e r'
removeRandomSubexpression (ExpressionArithmetic e1 e2 f s) r 
  | (nv == 0) && wh = ((ExpressionArithmetic GraftExpression e2 f s, Just e1), r'')
  | nv == 0 = ((ExpressionArithmetic e1 GraftExpression f s, Just e2), r'')
  | nv <= ebc1 = ((se, Just $ ExpressionArithmetic (fromJust rest) e2 f s), r''')
  | otherwise = ((se, Just $ ExpressionArithmetic e1 (fromJust rest) f s), r''')
  where ebc1 = expressionBranchCount e1
        ebc2 = expressionBranchCount e2
        (nv, r') = randomR (0, ebc1 + ebc2) r :: (Int, AES.AESRNG)
        (wh, r'') = random r' :: (Bool, AES.AESRNG)
        ((se, rest), r''') = removeRandomSubexpression (if (nv <= ebc1) then e1 else e2) r''
removeRandomSubexpression (ExpressionBranch e1 e2 f s e3 e4) r 
  | (nv == 0) && (wh == 0) = ((ExpressionBranch GraftExpression e2 f s e3 e4, Just e1), r'')
  | (nv == 0) && (wh == 1) = ((ExpressionBranch e1 GraftExpression f s e3 e4, Just e2), r'')
  | (nv == 0) && (wh == 2) = ((ExpressionBranch e1 e2 f s GraftExpression e4, Just e3), r'')
  | (nv == 0) = ((ExpressionBranch e1 e2 f s e3 GraftExpression, Just e4), r'')
  | nv <= ebc1 = ((se, Just $ ExpressionBranch (fromJust rest) e2 f s e3 e4), r''')
  | nv <= ebc1 + ebc2 = ((se, Just $ ExpressionBranch e1 (fromJust rest) f s e3 e4), r''')
  | nv <= ebc1 + ebc2 + ebc3 = ((se, Just $ ExpressionBranch e1 e2 f s (fromJust rest) e4), r''')
  | otherwise = ((se, Just $ ExpressionBranch e1 e2 f s e3 (fromJust rest)), r''')
  where ebc1 = expressionBranchCount e1
        ebc2 = expressionBranchCount e2
        ebc3 = expressionBranchCount e3
        ebc4 = expressionBranchCount e4
        (nv, r') = randomR (0, ebc1 + ebc2 + ebc3 + ebc4) r :: (Int, AES.AESRNG)
        (wh, r'') = randomR (0, 3) r' :: (Int, AES.AESRNG)
        ((se, rest), r''') = removeRandomSubexpression (if (nv <= ebc1) 
                                                        then e1 
                                                        else (if (nv <= ebc1 + ebc2) 
                                                              then e2 
                                                              else (if (nv <= ebc1 + ebc2 + ebc3) 
                                                                    then e3
                                                                    else e4))) r''
removeRandomSubexpression e r = ((e, Nothing), r)

removeRandomSubexpressionFromList :: [Expression] -> AES.AESRNG -> ((Expression, [Expression]), AES.AESRNG)
removeRandomSubexpressionFromList [] r = ((Value 133713371337, []), r)
removeRandomSubexpressionFromList l rng = rfl' nv l rng'
  where (nv, rng') = randomR (1, fromIntegral $ length l) rng :: (Int, AES.AESRNG)
        rfl' 1 (h:t) r
          | isNothing h' = ((se, t), r')
          | otherwise = ((se, (fromJust h'):t), r')
          where ((se, h'), r') = removeRandomSubexpression h r
        rfl' n (h:t) r = ((se, (h:t')), r')
          where ((se, t'), r') = rfl' (n - 1) t r

shrinkExpression :: Expression -> AES.AESRNG -> (Expression, AES.AESRNG)
shrinkExpression e r
  | b = (e, r')
  | (isNothing rest) = (e, r'')
  | otherwise = (e', r''')
  where (b, r') = random r :: (Bool, AES.AESRNG)
        ((_, rest), r'') = removeRandomSubexpression e r'
        (e', r''') = shrinkExpression (fromJust rest) r''

graftExpression :: Expression -> Expression -> (Bool, Expression)
graftExpression GraftExpression e = (True, e)
graftExpression (StackValue se) e = (v, StackValue se') 
  where (v, se') = graftExpression se e
graftExpression (InputWord se) e = (v, InputWord se')
  where (v, se') = graftExpression se e
graftExpression (ExpressionArithmetic se1 se2 f s) e = (v1 || v2, ExpressionArithmetic se1' se2' f s)
  where (v1, se1') = graftExpression se1 e
        (v2, se2') = graftExpression se2 e
graftExpression (ExpressionBranch se1 se2 t s se3 se4) e = (v1 || v2 || v3 || v4, ExpressionBranch se1' se2' t s se3' se4')
  where (v1, se1') = graftExpression se1 e
        (v2, se2') = graftExpression se2 e
        (v3, se3') = graftExpression se3 e
        (v4, se4') = graftExpression se4 e
graftExpression e _ = (False, e)

breedExpression :: [Expression] -> AES.AESRNG -> (Expression, AES.AESRNG)
breedExpression l rng = shrinkExpression e rng'
  where (e, rng') = be' l rng
        be' l rng
          | (Prelude.null l') || (not gr) = (se, rng')
          | otherwise = (ge, rng'')
          where ((se, l'), rng') = removeRandomSubexpressionFromList l rng
                (ben, rng'') = be' l' rng'
                (gr, ge) = graftExpression se ben

allExpressions :: Instruction -> [Expression]
allExpressions (Sequence i1 i2) = (allExpressions i1) ++ (allExpressions i2)
allExpressions (Insert e) = [e]
allExpressions (Delete e) = [e]
allExpressions (Move e) = [e]
allExpressions (Loop e1 e2 _ _ i) = [e1,e2] ++ (allExpressions i)
allExpressions (Branch e1 e2 _ _ i1 i2) = [e1,e2] ++ (allExpressions i1) ++ (allExpressions i2)
                
withExpressions :: Instruction -> [Expression] -> Instruction
withExpressions i l = ans
  where (ans, _) = we' i l
        we' (Sequence i1 i2) l = (Sequence i1' i2', l'')
          where (i1', l') = we' i1 l
                (i2', l'') = we' i2 l'
        we' (Insert _) (h:t) = (Insert h, t)
        we' (Delete _) (h:t) = (Delete h, t)
        we' (Move _) (h:t) = (Move h, t)
        we' (Loop _ _ tst s i) (e1:e2:t) = (Loop e1 e2 tst s i', l')
          where (i', l') = we' i t
        we' (Branch _ _ tst s i1 i2) (e1:e2:t) = (Branch e1 e2 tst s i1' i2', l'')
          where (i1', l') = we' i1 t
                (i2', l'') = we' i2 l'

removeRandomCompleteSubprogram :: Instruction -> AES.AESRNG -> ((Instruction, (Either Instruction Expression)), AES.AESRNG)
removeRandomCompleteSubprogram inst rng = ((gi inst', rest), rng')
  where (nv, rng') = randomR (1, totalBranchCount inst) rng :: (Int, AES.AESRNG)
        (oans, _) = rrcs' nv (Left inst)
        (inst', rest) = fromJust oans
        gi (Left i) = i
        ge (Right e) = e
        rrcs' :: Int -> (Either Instruction Expression) -> (Maybe (Either Instruction Expression, Either Instruction Expression), Int)
        rrcs' n (Left (Sequence i1 i2))
          | n == 1 = (Just (Left $ Sequence GraftInstruction i2, Left i1), 0)
          | n == 2 = (Just (Left $ Sequence i1 GraftInstruction, Left i2), 0)
          | isJust ans1 = (Just (Left $ Sequence (gi i1') i2, rest1), 0)
          | isJust ans2 = (Just (Left $ Sequence i1 (gi i2'), rest2), 0)
          | otherwise = (Nothing, 2 + c1 + c2)
          where (ans1, c1) = rrcs' (n - 2) (Left i1) 
                (ans2, c2) = rrcs' (n - (2 + c1)) (Left i2)
                (i1', rest1) = fromJust ans1
                (i2', rest2) = fromJust ans2
        rrcs' n (Left (Insert e))
          | n == 1 = (Just (Left $ Insert GraftExpression, Right e), 0)
          | isJust ans = (Just (Left $ Insert (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Delete e))
          | n == 1 = (Just (Left $ Delete GraftExpression, Right e), 0)
          | isJust ans = (Just (Left $ Delete (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Move e))
          | n == 1 = (Just (Left $ Move GraftExpression, Right e), 0)
          | isJust ans = (Just (Left $ Move (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Loop e1 e2 t s i))
          | n == 1 = (Just (Left $ Loop GraftExpression e2 t s i, Right e1), 0)
          | n == 2 = (Just (Left $ Loop e1 GraftExpression t s i, Right e2), 0)
          | n == 3 = (Just (Left $ Loop e1 e2 t s GraftInstruction, Left i), 0)
          | isJust ans1 = (Just (Left $ Loop (ge e1') e2 t s i, rest1), 0)
          | isJust ans2 = (Just (Left $ Loop e1 (ge e2') t s i, rest2), 0)
          | isJust ans3 = (Just (Left $ Loop e1 e2 t s (gi i'), rest3), 0)
          | otherwise = (Nothing, 3 + c1 + c2 + c3)
          where (ans1, c1) = rrcs' (n - 3) (Right e1) 
                (ans2, c2) = rrcs' (n - (3 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (3 + c1 + c2)) (Left i)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2
                (i', rest3) = fromJust ans3
        rrcs' n (Left (Branch e1 e2 t s i1 i2))
          | n == 1 = (Just (Left $ Branch GraftExpression e2 t s i1 i2, Right e1), 0)
          | n == 2 = (Just (Left $ Branch e1 GraftExpression t s i1 i2, Right e2), 0)
          | n == 3 = (Just (Left $ Branch e1 e2 t s GraftInstruction i2, Left i1), 0)
          | n == 4 = (Just (Left $ Branch e1 e2 t s i1 GraftInstruction, Left i2), 0)
          | isJust ans1 = (Just (Left $ Branch (ge e1') e2 t s i1 i2, rest1), 0)
          | isJust ans2 = (Just (Left $ Branch e1 (ge e2') t s i1 i2, rest2), 0)
          | isJust ans3 = (Just (Left $ Branch e1 e2 t s (gi i1') i2, rest3), 0)
          | isJust ans4 = (Just (Left $ Branch e1 e2 t s i1 (gi i2'), rest4), 0)
          | otherwise = (Nothing, 4 + c1 + c2 + c3 + c4)
          where (ans1, c1) = rrcs' (n - 4) (Right e1) 
                (ans2, c2) = rrcs' (n - (4 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (4 + c1 + c2)) (Left i1)
                (ans4, c4) = rrcs' (n - (4 + c1 + c2 + c3)) (Left i2)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2
                (i1', rest3) = fromJust ans3
                (i2', rest4) = fromJust ans4
        rrcs' n (Right (StackValue e))
          | n == 1 = (Just (Right $ StackValue GraftExpression, Right e), 0)
          | isJust ans = (Just (Right $ StackValue (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e)
                (e', rest) = fromJust ans
        rrcs' n (Right (InputWord e))
          | n == 1 = (Just (Right $ InputWord GraftExpression, Right e), 0)
          | isJust ans = (Just (Right $ InputWord (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e)
                (e', rest) = fromJust ans
        rrcs' n (Right (ExpressionArithmetic e1 e2 f s))
          | n == 1 = (Just (Right $ ExpressionArithmetic GraftExpression e2 f s, Right e1), 0)
          | n == 2 = (Just (Right $ ExpressionArithmetic e1 GraftExpression f s, Right e2), 0)
          | isJust ans1 = (Just (Right $ ExpressionArithmetic (ge e1') e2 f s, rest1), 0)
          | isJust ans2 = (Just (Right $ ExpressionArithmetic e1 (ge e2') f s, rest2), 0)
          | otherwise = (Nothing, 2 + c1 + c2)
          where (ans1, c1) = rrcs' (n - 2) (Right e1)
                (ans2, c2) = rrcs' (n - (2 + c1)) (Right e2)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2                
        rrcs' n (Right (ExpressionBranch e1 e2 f s e3 e4))
          | n == 1 = (Just (Right $ ExpressionBranch GraftExpression e2 f s e3 e4, Right e1), 0)
          | n == 2 = (Just (Right $ ExpressionBranch e1 GraftExpression f s e3 e4, Right e2), 0)
          | n == 3 = (Just (Right $ ExpressionBranch e1 e2 f s GraftExpression e4, Right e3), 0)
          | n == 4 = (Just (Right $ ExpressionBranch e1 e2 f s e3 GraftExpression, Right e4), 0)
          | isJust ans1 = (Just (Right $ ExpressionBranch (ge e1') e2 f s e3 e4, rest1), 0)
          | isJust ans2 = (Just (Right $ ExpressionBranch e1 (ge e2') f s e3 e4, rest2), 0)
          | isJust ans3 = (Just (Right $ ExpressionBranch e1 e2 f s (ge e3') e4, rest3), 0)
          | isJust ans4 = (Just (Right $ ExpressionBranch e1 e2 f s e3 (ge e4'), rest4), 0)
          | otherwise = (Nothing, 4 + c1 + c2 + c3 + c4)
          where (ans1, c1) = rrcs' (n - 4) (Right e1)
                (ans2, c2) = rrcs' (n - (4 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (4 + c1 + c2)) (Right e3)
                (ans4, c4) = rrcs' (n - (4 + c1 + c2 + c3)) (Right e4)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2                
                (e3', rest3) = fromJust ans3                
                (e4', rest4) = fromJust ans4                
        rrcs' n (Right e) = (Nothing, 0)
        

removeRandomCompleteSubexpression :: Instruction -> AES.AESRNG -> ((Instruction, Expression), AES.AESRNG)
removeRandomCompleteSubexpression inst rng = ((gi inst', rest), rng')
  where (nv, rng') = randomR (1, totalExpressionBranchCount inst) rng :: (Int, AES.AESRNG)
        (oans, _) = rrcs' nv (Left inst)
        (inst', rest) = fromJust oans
        gi (Left i) = i
        ge (Right e) = e
        rrcs' :: Int -> (Either Instruction Expression) -> (Maybe (Either Instruction Expression, Expression), Int)
        rrcs' n (Left (Sequence i1 i2))
          | isJust ans1 = (Just (Left $ Sequence (gi i1') i2, rest1), 0)
          | isJust ans2 = (Just (Left $ Sequence i1 (gi i2'), rest2), 0)
          | otherwise = (Nothing, c1 + c2)
          where (ans1, c1) = rrcs' n (Left i1) 
                (ans2, c2) = rrcs' (n - c1) (Left i2)
                (i1', rest1) = fromJust ans1
                (i2', rest2) = fromJust ans2
        rrcs' n (Left (Insert e))
          | n == 1 = (Just (Left $ Insert GraftExpression, e), 0)
          | isJust ans = (Just (Left $ Insert (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Delete e))
          | n == 1 = (Just (Left $ Delete GraftExpression, e), 0)
          | isJust ans = (Just (Left $ Delete (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Move e))
          | n == 1 = (Just (Left $ Move GraftExpression, e), 0)
          | isJust ans = (Just (Left $ Move (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e) 
                (e', rest) = fromJust ans
        rrcs' n (Left (Loop e1 e2 t s i))
          | n == 1 = (Just (Left $ Loop GraftExpression e2 t s i, e1), 0)
          | n == 2 = (Just (Left $ Loop e1 GraftExpression t s i, e2), 0)
          | isJust ans1 = (Just (Left $ Loop (ge e1') e2 t s i, rest1), 0)
          | isJust ans2 = (Just (Left $ Loop e1 (ge e2') t s i, rest2), 0)
          | isJust ans3 = (Just (Left $ Loop e1 e2 t s (gi i'), rest3), 0)
          | otherwise = (Nothing, 2 + c1 + c2 + c3)
          where (ans1, c1) = rrcs' (n - 2) (Right e1) 
                (ans2, c2) = rrcs' (n - (2 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (2 + c1 + c2)) (Left i)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2
                (i', rest3) = fromJust ans3
        rrcs' n (Left (Branch e1 e2 t s i1 i2))
          | n == 1 = (Just (Left $ Branch GraftExpression e2 t s i1 i2, e1), 0)
          | n == 2 = (Just (Left $ Branch e1 GraftExpression t s i1 i2, e2), 0)
          | isJust ans1 = (Just (Left $ Branch (ge e1') e2 t s i1 i2, rest1), 0)
          | isJust ans2 = (Just (Left $ Branch e1 (ge e2') t s i1 i2, rest2), 0)
          | isJust ans3 = (Just (Left $ Branch e1 e2 t s (gi i1') i2, rest3), 0)
          | isJust ans4 = (Just (Left $ Branch e1 e2 t s i1 (gi i2'), rest4), 0)
          | otherwise = (Nothing, 2 + c1 + c2 + c3 + c4)
          where (ans1, c1) = rrcs' (n - 2) (Right e1) 
                (ans2, c2) = rrcs' (n - (2 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (2 + c1 + c2)) (Left i1)
                (ans4, c4) = rrcs' (n - (2 + c1 + c2 + c3)) (Left i2)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2
                (i1', rest3) = fromJust ans3
                (i2', rest4) = fromJust ans4
        rrcs' n (Right (StackValue e))
          | n == 1 = (Just (Right $ StackValue GraftExpression, e), 0)
          | isJust ans = (Just (Right $ StackValue (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e)
                (e', rest) = fromJust ans
        rrcs' n (Right (InputWord e))
          | n == 1 = (Just (Right $ InputWord GraftExpression, e), 0)
          | isJust ans = (Just (Right $ InputWord (ge e'), rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) (Right e)
                (e', rest) = fromJust ans
        rrcs' n (Right (ExpressionArithmetic e1 e2 f s))
          | n == 1 = (Just (Right $ ExpressionArithmetic GraftExpression e2 f s, e1), 0)
          | n == 2 = (Just (Right $ ExpressionArithmetic e1 GraftExpression f s, e2), 0)
          | isJust ans1 = (Just (Right $ ExpressionArithmetic (ge e1') e2 f s, rest1), 0)
          | isJust ans2 = (Just (Right $ ExpressionArithmetic e1 (ge e2') f s, rest2), 0)
          | otherwise = (Nothing, 2 + c1 + c2)
          where (ans1, c1) = rrcs' (n - 2) (Right e1)
                (ans2, c2) = rrcs' (n - (2 + c1)) (Right e2)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2                
        rrcs' n (Right (ExpressionBranch e1 e2 f s e3 e4))
          | n == 1 = (Just (Right $ ExpressionBranch GraftExpression e2 f s e3 e4, e1), 0)
          | n == 2 = (Just (Right $ ExpressionBranch e1 GraftExpression f s e3 e4, e2), 0)
          | n == 3 = (Just (Right $ ExpressionBranch e1 e2 f s GraftExpression e4, e3), 0)
          | n == 4 = (Just (Right $ ExpressionBranch e1 e2 f s e3 GraftExpression, e4), 0)
          | isJust ans1 = (Just (Right $ ExpressionBranch (ge e1') e2 f s e3 e4, rest1), 0)
          | isJust ans2 = (Just (Right $ ExpressionBranch e1 (ge e2') f s e3 e4, rest2), 0)
          | isJust ans3 = (Just (Right $ ExpressionBranch e1 e2 f s (ge e3') e4, rest3), 0)
          | isJust ans4 = (Just (Right $ ExpressionBranch e1 e2 f s e3 (ge e4'), rest4), 0)
          | otherwise = (Nothing, 4 + c1 + c2 + c3 + c4)
          where (ans1, c1) = rrcs' (n - 4) (Right e1)
                (ans2, c2) = rrcs' (n - (4 + c1)) (Right e2)
                (ans3, c3) = rrcs' (n - (4 + c1 + c2)) (Right e3)
                (ans4, c4) = rrcs' (n - (4 + c1 + c2 + c3)) (Right e4)
                (e1', rest1) = fromJust ans1
                (e2', rest2) = fromJust ans2                
                (e3', rest3) = fromJust ans3                
                (e4', rest4) = fromJust ans4                
        rrcs' n (Right e) = (Nothing, 0)

removeRandomCompleteSubinstruction :: Instruction -> AES.AESRNG -> ((Instruction, Instruction), AES.AESRNG)
removeRandomCompleteSubinstruction inst rng
  | isJust oans = ((inst', rest), rng')
  | otherwise = ((GraftInstruction, inst), rng)
  where (nv, rng') = randomR (1, totalInstructionBranchCount inst) rng :: (Int, AES.AESRNG)
        (oans, _) = rrcs' nv inst
        (inst', rest) = fromJust oans
        rrcs' :: Int -> Instruction -> (Maybe (Instruction, Instruction), Int)
        rrcs' n (Sequence i1 i2)
          | n == 1 = (Just (Sequence GraftInstruction i2, i1), 0)
          | n == 2 = (Just (Sequence i1 GraftInstruction, i2), 0)
          | isJust ans1 = (Just (Sequence i1' i2, rest1), 0)
          | isJust ans2 = (Just (Sequence i1 i2', rest2), 0)
          | otherwise = (Nothing, 2 + c1 + c2)
          where (ans1, c1) = rrcs' (n - 2) i1
                (ans2, c2) = rrcs' (n - (2 + c1)) i2
                (i1', rest1) = fromJust ans1
                (i2', rest2) = fromJust ans2
        rrcs' n (Loop e1 e2 t s i)
          | n == 1 = (Just (Loop e1 e2 t s GraftInstruction, i), 0)
          | isJust ans = (Just (Loop e1 e2 t s i', rest), 0)
          | otherwise = (Nothing, 1 + c)
          where (ans, c) = rrcs' (n - 1) i
                (i', rest) = fromJust ans
        rrcs' n (Branch e1 e2 t s i1 i2)
          | n == 1 = (Just (Branch e1 e2 t s GraftInstruction i2, i1), 0)
          | n == 2 = (Just (Branch e1 e2 t s i1 GraftInstruction, i2), 0)
          | isJust ans1 = (Just (Branch e1 e2 t s i1' i2, rest1), 0)
          | isJust ans2 = (Just (Branch e1 e2 t s i1 i2', rest2), 0)
          | otherwise = (Nothing, 2 + c1 + c2)
          where (ans1, c1) = rrcs' (n - 2) i1
                (ans2, c2) = rrcs' (n - (2 + c1)) i2
                (i1', rest1) = fromJust ans1
                (i2', rest2) = fromJust ans2
        rrcs' n i = (Nothing, 0)

                           
monoMutateImprove :: Instruction -> (Array Int Int) -> ([Int] -> Integer) -> Integer -> Int -> Int -> AES.AESRNG -> (Instruction, AES.AESRNG)
monoMutateImprove inst input scorer n maxTicks maxStack rng = (inst', rng')
  where ((inst', _), rng') = mmi' inst instscr input scorer n maxTicks maxStack rng
        instscr = scorer (getStack (run inst input maxTicks maxStack))
        mmi' :: Instruction -> Integer -> (Array Int Int) -> ([Int] -> Integer) -> Integer -> Int -> Int -> AES.AESRNG -> ((Instruction, Integer), AES.AESRNG)
        mmi' i iscr _ _ 0 _ _ r = ((i, iscr), r)
        mmi' i iscr inp scrr n maxTicks maxStack r
          | iscr' > iscr = ((i', iscr'), r'')
          | otherwise = mmi' i iscr inp scrr (n - 1) maxTicks maxStack r''
          where (ri, r') = randomInstruction r
                (i', r'') = breedInstruction [i, ri] r'
                iscr' = scrr (getStack (run i' inp maxTicks maxStack)) 
            
data ProgramAndScore = ProgramAndScore { score::Integer, instruction::Instruction }

instance Eq ProgramAndScore where
  (ProgramAndScore s1 i1) == (ProgramAndScore s2 i2) = (s1 == s2) && (i1 == i2)

instance Ord ProgramAndScore where
  compare (ProgramAndScore s1 i1) (ProgramAndScore s2 i2)
    | (s1 == s2) && (is1 == is2) = compare i1 i2
    | (s1 == s2) && (is1 <= is2) = GT
    | s1 == s2 = LT
    | s1 <= s2 = LT
    | otherwise = GT
    where is1 = programSize i1
          is2 = programSize i2

programAndScoreToString :: ProgramAndScore -> String
programAndScoreToString (ProgramAndScore s p) = "---- Program: ----\n" ++ (instructionToString p) ++
                                                "\n---- Score: " ++ (show s) ++ " ----\n"

getProgramFromProgramAndScore (ProgramAndScore _ i) = i
getScoreFromProgramAndScore (ProgramAndScore s _) = s

genProgramPool :: Int -> ([Int] -> Integer) -> (Array Int Int) -> Int -> Int -> AES.AESRNG -> ((Set ProgramAndScore), AES.AESRNG)
genProgramPool size scorer input maxTicks maxStack rng = gpp' empty size scorer input maxTicks maxStack rng
  where gpp' :: (Set ProgramAndScore) -> Int -> ([Int] -> Integer) -> (Array Int Int) -> Int -> Int -> AES.AESRNG -> ((Set ProgramAndScore), AES.AESRNG)
        gpp' s sz scrr inp mt ms r
          | (Data.Set.size s) == sz = (s, r)
          | member npas s = gpp' s sz scrr inp mt ms r'
          | otherwise = gpp' (insert npas s) sz scrr inp mt ms r'
          where (ri, r') = randomInstruction r
                ps = run ri inp mt ms 
                npas = ProgramAndScore (scrr $ getStack ps) ri
                        
mutateImprove :: (Set ProgramAndScore) -> Integer -> (Array Int Int) -> ([Int] -> Integer) -> Int -> Int -> AES.AESRNG -> ((Maybe ProgramAndScore, Set ProgramAndScore), AES.AESRNG)
mutateImprove progp 0 input scorer maxTicks maxStack rng = ((Nothing, progp), rng)
mutateImprove progp attempts input scorer maxTicks maxStack rng
  | member nipas progp = mutateImprove progp attempts input scorer maxTicks maxStack rng'''''''
  | niscr > lowscore = ((Just nipas, progp'), rng''''''')
  | otherwise = mutateImprove progp (attempts - 1) input scorer maxTicks maxStack rng'''''''
  where (addrand, rng') = random rng :: (Bool, AES.AESRNG) 
        (nv, rng'') = randomR (1, 50) rng' :: (Int, AES.AESRNG)
        (nv2, rng''') = randomR (1, 50) rng'' :: (Int, AES.AESRNG)
        (ppl, rng'''') = pplg progpa nv rng'''
        (ril, rng''''') = gril nv2 rng''''
        (ppl', rng'''''') = if addrand then (ril ++ ppl, rng''''') else (ppl, rng'''')
        lowscore = score (findMin progp)
        (ni, rng''''''') = breedInstruction ppl' rng''''''
        niscr = scorer (getStack $ run ni input maxTicks maxStack)
        nipas = ProgramAndScore niscr ni
        progp' = insert nipas (deleteMin progp)
        progpa = listArray (1, Data.Set.size progp) (Prelude.map instruction (toList progp))
        pplg _ 0 r = ([], r)
        pplg a n r = ((a ! nv'):rest, r'')
          where (nv', r') = randomR (bounds a) r :: (Int, AES.AESRNG)
                (rest, r'') = pplg a (n - 1) r'
        gril 0 r = ([], r)
        gril n r = (h:t, r'')
          where (t, r') = gril (n - 1) r
                (h, r'') = randomInstruction r'
                
emptyInput :: Array Int Int
emptyInput = array (0,0) [(0,0)]

fibInput :: Array Int Int
fibInput = array (0, 15) [(0,0),(1,1),(2,1),(3,2),(4,3),(5,5),(6,8),(7,13),(8,21),(9,34),(10,55),(11,89),(12,144),(13,233),(14,377),(15,610)]

fib :: Instruction
fib =
 (Sequence (Insert (Value 1))
  (Sequence (Insert (Value 0))
   (Loop (StackValue (ExpressionArithmetic (StackSize) (Value 1) (\a b -> a - b) "-")) 
    (Value 100000000000) (\a b -> a < b) "<"
   (Sequence
    (Insert (ExpressionArithmetic (StackValue (ExpressionArithmetic (StackSize) (Value 1) (\a b -> a - b) "-"))
                    (StackValue (ExpressionArithmetic (StackSize)
                                     (Value 2) (\a b -> a - b) "-")) (\a b -> a+b) "+"))
           (Move (ExpressionArithmetic (StackSize) (Value 1) (\a b -> a - b) "-"))))))




scoreFib :: [Int] -> Integer
scoreFib l = sf' 0 1 l
  where sf' _ _ [] = 0
        sf' n m (h:t) = ((1000 - dst) + sf'') 
          where dst = (abs (n - (fromIntegral h)))
                sf'' = sf' m (n + m) t


{-mhelp _ n r = if ((getStackSize rp) >= 0) then ((instructionToString ri)  ++ "\n-----------------\n" ++ (mhelp 0 (n - 1) r')) else (mhelp 0 (n - 1) r')-}
{-mhelp n r = if ((getStackSize rp) >= 5) then ((programStateToString
rp) ++ "\n-----------------\n" ++ (mhelp (n - 1) r')) else (mhelp (n -
1) r')-}
{-mhelp s n r = if (fs > s) 
              then (instructionToString ri) ++ "\n" ++ (show fs) ++ "  " ++ (show $ getTicks rp) ++ "\n------\n" ++ (mhelp fs (n - 1) r') 
              else (mhelp s (n - 1) r')
 where (ri,r') = randomInstruction r
       rp = run ri emptyInput 100000 100
       fs = (scoreFib (reverse $ getStack rp))-}

mhelp3 0 r = ([],r)
mhelp3 n r 
  | expressionSize re >= 2 = (re:l,r'')
  | otherwise = mhelp3 n r'
  where (re, r') = randomExpression r
        (l, r'') = mhelp3 (n - 1) r'
mhelp3i 0 r = ([],r)
mhelp3i n r 
  | instructionBranchCount re >= 1 = (re:l,r'')
  | otherwise = mhelp3i n r'
  where (re, r') = randomInstruction r
        (l, r'') = mhelp3i (n - 1) r'

mhelp _ 0 _ = ""
mhelp _ n r = (foldr (\a b -> (expressionToString a) ++ "\n---\n" ++ b) "" l) ++
              "\n----------------------\n" ++ 
              mhelp2 50 l r ++ mhelp 0 (n - 1) r'
  where (l,r') = mhelp3 2 r

mhelp2 0 _ _ = ""
mhelp2 _ [] _ = ""
mhelp2 n l r = "Bred: " ++ (expressionToString el) ++ "\n----\n" ++
               mhelp2 (n - 1) l r' 
  where (el, r') = breedExpression l r
               
mhelp4 0 _ = ""
mhelp4 n r = (expressionToString re) ++ "\n-------------\n" ++ mhelp5 re 50 r ++ mhelp4 (n - 1) r'
  where (re, r') = randomExpression r

mhelp5 _ 0 _ = ""
mhelp5 e n r = "took: " ++ (expressionToString re) ++ "\nleft: " ++ (if (isJust rest) then (expressionToString $ fromJust rest) else "nada") ++ "\n--\n" ++ mhelp5 e (n - 1) r'
  where ((re, rest), r') = removeRandomSubexpression e r

mhelp7 [] _ = ""
mhelp7 l r = "Removed: " ++ (instructionToString re) ++ "\nleft:\n" ++ 
             (foldr (\a b -> (instructionToString a) ++ "\n-5-\n" ++ b) "" l')
            ++ "\n\n--6-----\n" 
            ++ mhelp7 l' r'
  where ((re, l'), r') = removeRandomSubinstructionFromList l r

mhelp8 _ 0 _ = ""
mhelp8 _ n r = (foldr (\a b -> (instructionToString a) ++ "\n---\n" ++ b) "" l) ++
              "\n----------------------\n" ++ 
              mhelp9 150 l r ++ mhelp8 0 (n - 1) r'
  where (l,r') = mhelp3i 3 r

mhelp9 0 _ _ = ""
mhelp9 _ [] _ = ""
mhelp9 n l r = "Bred: " ++ (instructionToString el) ++ "\n----\n" ++
               mhelp9 (n - 1) l r' 
  where (el, r') = breedInstruction l r

mhelp6 [] _ = ""
mhelp6 l r = "Removed: " ++ (expressionToString re) ++ "\nleft:\n" ++ 
             (foldr (\a b -> (expressionToString a) ++ "\n-3-\n" ++ b) "" l')
            ++ "\n\n--2-----\n" 
            ++ mhelp6 l' r'
  where ((re, l'), r') = removeRandomSubexpressionFromList l r


          
mhelp10 0 r = ""
mhelp10 n r = (instructionToString i) ++ "\n---------inst-----------\n" ++
              (instructionToString $ withExpressions i l) ++ "\n---------inst-----------\n" ++
              (foldr (\a b -> (expressionToString a) ++ "\n--expr--\n" ++ b) "" l) ++
              mhelp10 (n - 1) r'
  where l = allExpressions i
        (i, r') = randomInstruction r


mhelp11 _ 0 _ = ""
mhelp11 s n r = if (fs > s) 
              then (instructionToString ri) ++ "\n" ++ (show fs) ++ "  " ++ (show $ getTicks rp) ++ "\n------\n" ++ 
                   (mhelp11 fs (n - 1) r') 
              else (mhelp11 s (n - 1) r')
 where (ri,r') = randomInstruction r
       rp = run ri emptyInput 100000 100
       fs = (scoreFib (reverse $ getStack rp))


mhelp12 i n r
  | ni == i = (instructionToString i) ++ "\n--done--\n"
  | otherwise = (instructionToString ni) ++ "\n------" ++ (show scr) ++ "-------\n" ++ mhelp12 ni n r'
  where (ni, r') = monoMutateImprove i emptyInput scoreFib 200000000000000000000000000000000000000000000000000000000000000 n 100 r
        scr = scoreFib $ getStack $ run ni emptyInput 10000 100


mhelp13 0 _ = ""
mhelp13 n r = (instructionToString ri) ++ "\n------------1---------------\n" ++ mhelp14 40 ri r ++ 
              mhelp13 (n - 1) r'
              where (ri, r') = randomInstruction r
                    
mhelp14 0 i _ = ""
mhelp14 n i r = "Top:\n" ++ (instructionToString t) ++ "\nBottom:\n" ++ (instructionToString re) ++ "\n---14---\n" ++
                mhelp14 (n - 1) i r'
                where ((t, re), r') = removeRandomCompleteSubinstruction i r
                      mh' (Left i) = instructionToString i
                      mh' (Right e) = expressionToString e
  

                    

mhelp15 n r = (fold (\e s -> (programAndScoreToString e) ++ s) "" pp) ++ 
              "\n------------------------------\n\n\n" ++ (mhelp16 pp n r')
              where (pp, r') = (genProgramPool 250 scoreFib fibInput 30000 100 r)

mhelp16 _ 0 r = ""
mhelp16 pp n r 
  | isJust pas = "\n---- found on attempt: " ++ (show n) ++ " ----\n" ++ (programAndScoreToString $ fromJust pas) ++ 
                 "\n---- HISCORE: " ++ (show $ score $ findMax pp') ++ " ----\n" ++
                 (mhelp16 pp' (n - 1) r')
  | otherwise = "\n---- attempt: " ++ (show n) ++ " ----\n" ++ 
                (programAndScoreToString $ findMax pp') ++
                (mhelp16 pp (n - 1) r')
  where ((pas, pp'), r') = mutateImprove pp 100 fibInput scoreFib 30000 100 r
               
main = do putStrLn "Enter a number:"
          x <- readLn
          putStrLn $ mhelp15 x rnd
          
{-

0  1
0  0  1  1  2  3  4  5
0  0  0  0  1  1  1  1  2  2  3  3  4  4  5  5  6  7  8  9  10 11 12 13
0  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1  2  2  2  2  3  3  3  3  4  4  4  4  5  5  5  5  6  6  7  7  8  89  10 11 12 13
--foo
-}

db [] = []
db (h:t) = (h:h:(db t))

sq 1 = [0, 1]
sq n = (db (sq (n - 1))) ++ [ (2 ^ n - 2) .. (2 ^ (n + 1) - 3) ]
  
avg :: [Integer] -> Double  
avg l = (fromIntegral $ sum l) / (fromIntegral $ length l)


ab :: Integer -> Rational
ab m = (2 * n + (2 ^ m - 2) * (n + 1)) / (2 ^ m)
  where n = fromIntegral m
        
ss :: Integer -> Rational
ss 0 = 0
ss n = ((ab n) / (2 ^ n)) + ss (n - 1)