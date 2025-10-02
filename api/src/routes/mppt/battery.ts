import express, { Request, Response } from 'express'

const router = express.Router()

router.post('/battery', async (req: Request, res: Response) => {
    req.app.get('io').emit('battery', req.body);
    res.status(200).send()
})





export { router as batteryRoute }