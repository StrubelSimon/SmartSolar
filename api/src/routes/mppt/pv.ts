import express, { Request, Response } from 'express'

const router = express.Router()

router.post('/pv', async (req: Request, res: Response) => {
    // console.log(req.body);
    req.app.get('io').emit('pv', req.body);
    res.status(201).send()
})


export { router as pvRoute }